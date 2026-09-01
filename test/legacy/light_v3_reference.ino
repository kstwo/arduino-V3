#include <avr/wdt.h>

// IRremote на таймере 1, иначе конфликт с tone()
#define IR_USE_AVR_TIMER1
#include <IRremote.hpp>

#define DEBUG 1
#define USE_WDT 1
#define RELAY_LOW 1 
#define BAUD 115200

// пины
#define IR_PIN 2
#define MIC_PIN 3
#define RELAY_PIN 4
#define LED_PIN 5
#define BUZ_PIN 6
#define BTN_LIGHT 7
#define BTN_PWR 8
#define BTN_MUTE 9
#define BTN_NIGHT 10

// тайминги
#define IR_DB 250
#define CLAP_DB 250
#define CLAP_WIN 700
#define RELAY_PROT 500 
#define BTN_DB 50
#define DBL_CLICK 350

// пульт
#define IR_MUTE 0x09
#define IR_L_ON 0x15
#define IR_L_OFF 0x07

#define IR_1M 0x16
#define IR_5M 0x19
#define IR_10M 0x0D
#define IR_15M 0x0C
#define IR_20M 0x18
#define IR_30M 0x5E
#define IR_1H 0x08
#define IR_2H 0x5A
#define IR_3H 0x42
#define IR_4H 0x52
#define IR_5H 0x1C
#define IR_8H 0x4A

// частоты
#define REST 0
#define C5 523
#define D5 587
#define E5 659
#define F5 698
#define G4 392
#define G5 784
#define A5 880
#define C6 1047
#define E6 1319
#define G6 1568

struct Note {
    uint16_t freq;
    uint16_t dur;
};

// звуки
const Note snd_mario[] PROGMEM = {
    {E5, 100}, {E5, 100}, {REST, 100}, {E5, 100},
    {REST, 100}, {C5, 100}, {E5, 200},
    {G5, 200}, {REST, 200}, {G4, 200} 
};

const Note snd_l_on[] PROGMEM = { {C6, 60}, {E6, 60}, {REST, 40}, {G6, 120} };
const Note snd_l_off[] PROGMEM = { {A5, 60}, {C6, 60}, {REST, 40}, {E6, 120} };
const Note snd_m_on[] PROGMEM = { {G4, 200} }; 
const Note snd_m_off[] PROGMEM = { {C6, 200} };
const Note snd_timer[] PROGMEM = { {C5, 60}, {F5, 60}, {G5, 120} };
const Note snd_click[] PROGMEM = { {C6, 40} };

// эффекты
enum Fx {
    FX_NONE = 0,
    FX_FLASH,
    FX_DBL,
    FX_FADE
};

struct Btn {
    uint8_t pin;
    bool last_raw;
    bool pressed;
    uint32_t db_time;
    uint8_t clicks;
    uint32_t rel_time;
};

struct State {
    bool pwr;
    bool muted;
    bool night;
    bool light;
    bool timer_act;
    bool playing;

    uint32_t t_dur;
    uint32_t t_start;

    uint32_t last_clap;
    uint32_t first_clap;
    uint32_t r_click_time;
    bool wait_clap2;
    bool mic_last;

    uint32_t last_ir;

    const Note* mel;
    uint16_t m_len;
    uint16_t m_idx;
    uint32_t n_start;

    Fx fx;
    uint32_t fx_start;
};

State st;

// кнопки
Btn btn_pwr = {BTN_PWR, HIGH, false, 0, 0, 0};
Btn btn_light = {BTN_LIGHT, HIGH, false, 0, 0, 0};
Btn btn_mute = {BTN_MUTE, HIGH, false, 0, 0, 0};
Btn btn_night = {BTN_NIGHT, HIGH, false, 0, 0, 0};

#if DEBUG
  #define DBG(x) Serial.print(x)
  #define DBGLN(x) Serial.println(x)
#else
  #define DBG(x)
  #define DBGLN(x)
#endif

// прототипы
void set_relay(bool s);
void set_light(bool s);
void set_timer(uint16_t m);
void cancel_timer();

void check_ir();
void check_clap();
void check_btns();
void poll_btn(Btn& b);
void check_tmr();

void play_snd(const Note* m, uint16_t len);
void tick_buzz();
void start_fx(Fx f);
void tick_fx();

void sleep_sys();
void wake_sys();
void reboot();

void setup() {
    pinMode(MIC_PIN, INPUT_PULLUP);   
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZ_PIN, OUTPUT);
    
    pinMode(BTN_LIGHT, INPUT_PULLUP);
    pinMode(BTN_PWR, INPUT_PULLUP);
    pinMode(BTN_MUTE, INPUT_PULLUP);
    pinMode(BTN_NIGHT, INPUT_PULLUP);

    set_relay(false);
    digitalWrite(LED_PIN, LOW);
    noTone(BUZ_PIN);

    Serial.begin(BAUD);

    st.pwr = true;
    st.mic_last = (digitalRead(MIC_PIN) == HIGH);
    
    btn_pwr.last_raw = !digitalRead(BTN_PWR);
    btn_light.last_raw = !digitalRead(BTN_LIGHT);
    btn_mute.last_raw = !digitalRead(BTN_MUTE);
    btn_night.last_raw = !digitalRead(BTN_NIGHT);

    IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK);
    play_snd(snd_mario, sizeof(snd_mario)/sizeof(Note));

#if USE_WDT
    wdt_enable(WDTO_2S);        
#endif
    DBGLN(F("init ok"));
}

void loop() {
#if USE_WDT
    wdt_reset();                         
#endif

    check_btns();
    if (!st.pwr) return;

    tick_buzz();                      
    tick_fx();                  
    check_ir();                          
    check_clap();                        

    if (st.timer_act) check_tmr();

    if (st.wait_clap2 && (millis() - st.first_clap > CLAP_WIN)) {
        st.wait_clap2 = false;
    }
}

void poll_btn(Btn& b) {
    bool raw = !digitalRead(b.pin); 
    uint32_t ms = millis();

    if (raw != b.last_raw) b.db_time = ms;
    b.last_raw = raw;

    if (ms - b.db_time > BTN_DB) {
        if (raw != b.pressed) {
            b.pressed = raw;
            if (!b.pressed) { 
                if (b.clicks == 0 || ms - b.rel_time > DBL_CLICK) b.clicks = 1;
                else b.clicks++;
                b.rel_time = ms;
            }
        }
    }
}

void check_btns() {
    poll_btn(btn_pwr);
    if (btn_pwr.clicks > 0 && !btn_pwr.pressed) {
        if (st.timer_act) {
            cancel_timer();
            play_snd(snd_click, 1);
            start_fx(FX_FLASH);
        }
        btn_pwr.clicks = 0;
    }

    if (!st.pwr) return;

    poll_btn(btn_light);
    if (btn_light.clicks > 0 && !btn_light.pressed) {
        st.light = !st.light;
        set_light(st.light);
        btn_light.clicks = 0;
    }

    poll_btn(btn_mute);
    if (btn_mute.clicks > 0 && !btn_mute.pressed) {
        st.muted = !st.muted;
        play_snd(st.muted ? snd_m_on : snd_m_off, 1);
        btn_mute.clicks = 0;
    }

    poll_btn(btn_night);
    if (btn_night.clicks > 0 && !btn_night.pressed) {
        st.night = !st.night;
        if (st.night) {
            noTone(BUZ_PIN);
            analogWrite(LED_PIN, 0);
            st.playing = false;
        } else {
            play_snd(snd_m_off, 1);
        }
        btn_night.clicks = 0;
    }
}

void reboot() {
    noTone(BUZ_PIN);
    analogWrite(LED_PIN, 0);
    set_relay(false);
    Serial.flush();
    wdt_enable(WDTO_15MS);
    while (1); 
}

void sleep_sys() {
    cancel_timer();
    set_relay(false);
    st.light = false;
    st.pwr = false;
    st.playing = false;
    noTone(BUZ_PIN);
    analogWrite(LED_PIN, 0);
#if USE_WDT
    wdt_disable();
#endif
}

void wake_sys() {
    st.pwr = true;
#if USE_WDT
    wdt_enable(WDTO_2S);
#endif
    play_snd(snd_l_on, sizeof(snd_l_on)/sizeof(Note));
    start_fx(FX_DBL);
}

void check_ir() {
    if (!IrReceiver.decode()) return;

    bool rep = IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT;

    if (!rep && (millis() - st.last_ir > IR_DB)) {
        uint16_t cmd = IrReceiver.decodedIRData.command;
        bool is_t = false;

        if (cmd == IR_MUTE) {
            st.muted = !st.muted;
            play_snd(st.muted ? snd_m_on : snd_m_off, 1);
        } 
        else if (cmd == IR_L_ON) { cancel_timer(); st.light = true; set_light(true); }
        else if (cmd == IR_L_OFF) { cancel_timer(); st.light = false; set_light(false); }
        else if (cmd == IR_1M)  { set_timer(1); is_t=true; }
        else if (cmd == IR_5M)  { set_timer(5); is_t=true; }
        else if (cmd == IR_10M) { set_timer(10); is_t=true; }
        else if (cmd == IR_15M) { set_timer(15); is_t=true; }
        else if (cmd == IR_20M) { set_timer(20); is_t=true; }
        else if (cmd == IR_30M) { set_timer(30); is_t=true; }
        else if (cmd == IR_1H)  { set_timer(60); is_t=true; }
        else if (cmd == IR_2H)  { set_timer(120); is_t=true; }
        else if (cmd == IR_3H)  { set_timer(180); is_t=true; }
        else if (cmd == IR_4H)  { set_timer(240); is_t=true; }
        else if (cmd == IR_5H)  { set_timer(300); is_t=true; }
        else if (cmd == IR_8H)  { set_timer(480); is_t=true; }

        if (is_t) play_snd(snd_timer, sizeof(snd_timer)/sizeof(Note));
        st.last_ir = millis();
    }
    IrReceiver.resume();
}

void check_clap() {
    bool mic = (digitalRead(MIC_PIN) == HIGH);

    // игнор
    if (st.muted || (millis() - st.r_click_time < RELAY_PROT)) {
        st.mic_last = mic;
        return;
    }

    if (st.mic_last && !mic) {
        uint32_t ms = millis();
        if (ms - st.last_clap >= CLAP_DB) {
            st.last_clap = ms;

            if (!st.wait_clap2) {
                st.first_clap = ms;
                st.wait_clap2 = true;
                start_fx(FX_FLASH);   
            } 
            else {
                if (ms - st.first_clap <= CLAP_WIN) {
                    cancel_timer(); 
                    st.light = !st.light;
                    set_light(st.light);
                    st.wait_clap2 = false;
                } else {
                    // сброс
                    st.first_clap = ms;
                    start_fx(FX_FLASH);
                }
            }
        }
    }
    st.mic_last = mic;
}

void check_tmr() {
    if (millis() - st.t_start >= st.t_dur) {
        cancel_timer();
        st.light = !st.light;
        set_light(st.light);
    }
}

void set_light(bool s) {
    set_relay(s);
    st.r_click_time = millis(); 

    if (s) {
        play_snd(snd_l_on, sizeof(snd_l_on)/sizeof(Note));
        start_fx(FX_DBL);  
    } else {
        play_snd(snd_l_off, sizeof(snd_l_off)/sizeof(Note));
        start_fx(FX_FADE);      
    }
}

void set_relay(bool s) {
#if RELAY_LOW
    digitalWrite(RELAY_PIN, s ? LOW : HIGH);
#else
    digitalWrite(RELAY_PIN, s ? HIGH : LOW);
#endif
}

void set_timer(uint16_t m) {
    st.t_dur = (uint32_t)m * 60000UL;
    st.t_start = millis();
    st.timer_act = true;
    st.fx = FX_NONE;
}

void cancel_timer() {
    if (st.timer_act) {
        st.timer_act = false;
        st.fx = FX_NONE;
    }
}

void play_snd(const Note* m, uint16_t len) {
    if (len == 0 || !m || !st.pwr || st.night) return;

    st.mel = m;
    st.m_len = len;
    st.m_idx = 0;
    st.n_start = millis();
    st.playing = true;

    uint16_t f = pgm_read_word(&m[0].freq);
    if (f > 0) tone(BUZ_PIN, f);
    else noTone(BUZ_PIN);
}

void tick_buzz() {
    if (!st.playing || st.night) return;

    uint32_t ms = millis();
    uint16_t dur = pgm_read_word(&st.mel[st.m_idx].dur);

    if (ms - st.n_start >= dur) {
        st.m_idx++;
        if (st.m_idx >= st.m_len) {
            st.playing = false;
            noTone(BUZ_PIN);
            return;
        }
        st.n_start = ms; 
        
        uint16_t f = pgm_read_word(&st.mel[st.m_idx].freq);
        if (f > 0) tone(BUZ_PIN, f);
        else noTone(BUZ_PIN);  
    }
}

void start_fx(Fx f) {
    if (st.night) return;
    st.fx = f;
    st.fx_start = millis();
}

void tick_fx() {
    if (st.night) {
        analogWrite(LED_PIN, 0);
        return;
    }

    uint32_t ms = millis();
    uint32_t dt = ms - st.fx_start;

    switch (st.fx) {
        case FX_FLASH:
            if (dt < 150) analogWrite(LED_PIN, 255);
            else { analogWrite(LED_PIN, 0); st.fx = FX_NONE; }
            break;

        case FX_DBL:
            if (dt < 100) analogWrite(LED_PIN, 255);
            else if (dt < 180) analogWrite(LED_PIN, 0);
            else if (dt < 280) analogWrite(LED_PIN, 255);
            else { analogWrite(LED_PIN, 0); st.fx = FX_NONE; }
            break;

        case FX_FADE:
            if (dt < 800) {
                int pwm = 255 - (dt * 255 / 800);
                analogWrite(LED_PIN, pwm);
            } else {
                analogWrite(LED_PIN, 0);
                st.fx = FX_NONE;
            }
            break;

        default:
            if (st.timer_act) {
                analogWrite(LED_PIN, ((ms / 1000) % 2) ? 0 : 255);
            }
            else if (st.muted) {
                uint16_t ph = ms % 2000;
                int pwm = (ph < 1000) ? (ph * 255 / 1000) : (255 - ((ph - 1000) * 255 / 1000));
                analogWrite(LED_PIN, pwm);
            }
            else {
                analogWrite(LED_PIN, 0);  
            }
            break;
    }
}
