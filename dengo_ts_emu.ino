//====================================================================
//  電車でGO!コントローラでTSマスコンをエミュレートしてBVEで遊ぶ。
//  2021.06.01 7M4MON
//  https://github.com/KeKe115/Densya_de_go
//  を元に全面的に処理を見直し。
//  
// TSマスコンのコマンドとBVEの内部処理の解説
// https://scrapbox.io/p4ken/%E5%85%A5%E5%8A%9B%E3%83%87%E3%83%90%E3%82%A4%E3%82%B9%E3%83%97%E3%83%A9%E3%82%B0%E3%82%A4%E3%83%B3%E5%88%B6%E4%BD%9C%E8%AC%9B%E5%BA%A7
//====================================================================

// Gyokimae氏の自作ライブラリ(PSコントローラ用)
// http://pspunch.com/pd/article/arduino_lib_gpsx.html
#include <GPSXClass.h>

//====================================================================
//  シリアルコマンドの定義
//====================================================================

// 受ける側がブレーキとノッチを同時に入れることを想定していない。
// TSA50 はブレーキが解除されたのか、ノッチが解除されたのか判断できない。
const char ts_cmd_handle[15][6] = { "TSB20", "TSB30", "TSB40", "TSE99", "TSA05", "TSA15", "TSA25", "TSA35", "TSA45", "TSA50", "TSA55", "TSA65", "TSA75", "TSA85","TSA95"};
const char ts_cmd_button_a[2][6] = {"TSX00", "TSX99"};
const char ts_cmd_button_b[2][6] = {"TSY00", "TSY99"};
const char ts_cmd_button_c[2][6] = {"TSZ00", "TSZ99"};
const char ts_cmd_button_start[2][6] = {"TSK00", "TSK99"};
const char ts_cmd_button_reverser[3][6] = {"TSG50", "TSG99", "TSG00"};  // 中立, 前, 後
const uint8_t reverser_state [4] = {0, 1, 0, 2};    // セレクトボタンで状態遷移 中立→前進→中立→後退


typedef struct
{
    uint8_t notch;
    uint8_t brake;
    uint8_t handle;
    bool button_a;
    bool button_b;
    bool button_c;
    bool button_start;
    bool button_select;
} MasconState_t;



//====================================================================
//  初期化処理
//====================================================================
void setup()
{
    // TSマスコンは 19200bps
    Serial.begin(19200);

    // PSコントローラライブラリの初期化
    PSX.mode(PSX_PAD1, MODE_DIGITAL, MODE_LOCK);
    PSX.motorEnable(PSX_PAD1, MOTOR1_DISABLE, MOTOR2_DISABLE);

    // レバーサの初期状態を送信
    Serial.print("TSG50\r");

}

//====================================================================
//  メインループ
//====================================================================
void loop()
{
    static uint8_t reverser_position = 0;
    static MasconState_t last_mascon_state = {0xff,0xff,0xff,false,false,false,false,false};
    MasconState_t mascon_state = get_mascon_state();

    // コントローラの状態に応じてTSマスコンのコマンドを投げる
    if ((last_mascon_state.handle != mascon_state.handle) && mascon_state.handle != 0xff){        //前回から変わったとき、かつ正常に取得できたときだけ。
        Serial.print(ts_cmd_handle[mascon_state.handle]);
        Serial.print("\r");
        last_mascon_state.handle = mascon_state.handle;
    }
    if (last_mascon_state.button_a != mascon_state.button_a){
        Serial.print(ts_cmd_button_a[mascon_state.button_a]);   // bool 型から int 型への変換では false→0, true→1 が保証されている。
        Serial.print("\r");
        last_mascon_state.button_a = mascon_state.button_a;
    }
    if (last_mascon_state.button_b != mascon_state.button_b){
        Serial.print(ts_cmd_button_b[mascon_state.button_b]);   
        Serial.print("\r");
        last_mascon_state.button_b = mascon_state.button_b;
    }
    if (last_mascon_state.button_c != mascon_state.button_c){
        Serial.print(ts_cmd_button_c[mascon_state.button_c]);   
        Serial.print("\r");
        last_mascon_state.button_c = mascon_state.button_c;
    }
    if (last_mascon_state.button_start != mascon_state.button_start){
        Serial.print(ts_cmd_button_start[mascon_state.button_start]);   
        Serial.print("\r");
        last_mascon_state.button_start = mascon_state.button_start;
    }
    if (last_mascon_state.button_select != mascon_state.button_select){     //セレクトは押されるたびにレバーサの状態を変える
        if (mascon_state.button_select){        //押されたとき
            reverser_position++;
            if (reverser_position > 3) reverser_position = 0;
            Serial.print(ts_cmd_button_reverser[reverser_state[reverser_position]]);
            Serial.print("\r");
        }else{   // 離されたとき
            ;    // 処理なし
        }
        last_mascon_state.button_select = mascon_state.button_select;
    }
    delay(20);      // ポーリング間隔が65ms以上開くとワンハンドルタイプではリセットされる。(2~60がOK範囲)
}


//====================================================================
//  マスコン状態の取得関数
//====================================================================

#define HANDLE_CONTACT_1 0b0001
#define HANDLE_CONTACT_2 0b0010
#define HANDLE_CONTACT_3 0b0100
#define HANDLE_CONTACT_4 0b1000

#define MAP_NOTCH_OFF 	0b0111
#define MAP_NOTCH_1		0b1110
#define MAP_NOTCH_2 	0b0110
#define MAP_NOTCH_3		0b1011
#define MAP_NOTCH_4		0b0011
#define MAP_NOTCH_5		0b1010

#define MAP_BRAKE_OFF 	0b1101
#define MAP_BRAKE_1		0b0111
#define MAP_BRAKE_2 	0b0101
#define MAP_BRAKE_3		0b1110
#define MAP_BRAKE_4		0b1100
#define MAP_BRAKE_5		0b0110
#define MAP_BRAKE_6		0b0100
#define MAP_BRAKE_7 	0b1011
#define MAP_BRAKE_8		0b1001
#define MAP_BRAKE_EMER		0b0000

uint8_t get_notch_state(uint8_t notch_position){
    uint8_t notch_state = 0xff;
    switch (notch_position){
        case MAP_NOTCH_OFF : notch_state = 0; break;
        case MAP_NOTCH_1 :	notch_state = 1; break;
        case MAP_NOTCH_2 :	notch_state = 2; break;
        case MAP_NOTCH_3 :	notch_state = 3; break;
        case MAP_NOTCH_4 :	notch_state = 4; break;
        case MAP_NOTCH_5 :	notch_state = 5; break;
        default : break;    //中途半端な状態は取得しない。
    }
    return notch_state;
}

uint8_t get_brake_state(uint8_t brake_position){
    uint8_t brake_state = 0xff;
    switch (brake_position){
        case MAP_BRAKE_OFF :	brake_state = 0; break;
        case MAP_BRAKE_1 :	brake_state = 1; break;
        case MAP_BRAKE_2 :	brake_state = 2; break;
        case MAP_BRAKE_3 :	brake_state = 3; break;
        case MAP_BRAKE_4 :	brake_state = 4; break;
        case MAP_BRAKE_5 :	brake_state = 5; break;
        case MAP_BRAKE_6 :	brake_state = 6; break;
        case MAP_BRAKE_7 :	brake_state = 7; break;
        case MAP_BRAKE_8 :	brake_state = 8; break;
        case MAP_BRAKE_EMER	:	brake_state = 9; break;
        default : break;    //中途半端な状態は取得しない。
    }
    return brake_state;
}

MasconState_t get_mascon_state(){
    // 戻り値を初期化
    MasconState_t current_mascon_state = {0xff,0xff,0xff,false,false,false,false,false};

    // PSコントローラの状態更新
    PSX.updateState(PSX_PAD1);

    // ノッチ状態の取得
    uint8_t notch_position = 0;
    if (IS_DOWN_LEFT(PSX_PAD1)) notch_position |= HANDLE_CONTACT_1;
    if (IS_DOWN_DOWN(PSX_PAD1)) notch_position |= HANDLE_CONTACT_2;
    if (IS_DOWN_RIGHT(PSX_PAD1)) notch_position |= HANDLE_CONTACT_3;
    if (IS_DOWN_TRIANGLE(PSX_PAD1)) notch_position |= HANDLE_CONTACT_4;
    current_mascon_state.notch = get_notch_state(notch_position);

    // ブレーキ状態の取得
    uint8_t brake_position = 0;
    if (IS_DOWN_R1(PSX_PAD1)) brake_position |= HANDLE_CONTACT_1;
    if (IS_DOWN_L1(PSX_PAD1)) brake_position |= HANDLE_CONTACT_2;
    if (IS_DOWN_R2(PSX_PAD1)) brake_position |= HANDLE_CONTACT_3;
    if (IS_DOWN_L2(PSX_PAD1)) brake_position |= HANDLE_CONTACT_4;
    current_mascon_state.brake = get_brake_state(brake_position);

    // ハンドルポジションは正常に取得できた場合のみ更新。
    if (current_mascon_state.brake != 0xff && current_mascon_state.notch != 0xff ){
        // ブレーキが入っていない場合のみノッチを返す。ブレーキ解除が9なのでオフセットする。
        current_mascon_state.handle = current_mascon_state.brake ? 9 - current_mascon_state.brake : current_mascon_state.notch + 9;
    }else{
        current_mascon_state.handle = 0xff;
    }

    //　ボタン状態の取得
    if (IS_DOWN_SQUARE(PSX_PAD1)) current_mascon_state.button_a = true;
    if (IS_DOWN_CROSS(PSX_PAD1)) current_mascon_state.button_b = true;
    if (IS_DOWN_CIRCLE(PSX_PAD1)) current_mascon_state.button_c = true;
    if (IS_DOWN_START(PSX_PAD1)) current_mascon_state.button_start = true;
    if (IS_DOWN_SELECT(PSX_PAD1)) current_mascon_state.button_select = true;

    return current_mascon_state;
}

