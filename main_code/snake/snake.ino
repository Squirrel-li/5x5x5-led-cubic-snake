/*
*電子112甲 專題 3D立體貪食蛇
*LED三維矩陣、搖桿、按鈕、LCD
*
*
*LCD顯示資料
*
*begin
*0123456789abcdef
*3-D snake game
*botton to play
*
*playing
*0123456789abcdef
*playing
*score:      xxx
*      123456789
*
*win
*0123456789abcdef
*new record!!!!  
*
*score:       xxx
*      0123456789
*
*
*lose
*0123456789abcdef
*score:       xxx
*high record: xxx
*      0123456789
*
*前後左右上下 012345
*
*timer0 系統
*timer1 LED掃描
*timer3 蘋果閃爍
*timer4 debug 顯示陣列輸出
*timer5 debug 接收搖桿訊號
*
*timer1~6 16bit resgister when TCNT = 65535 + 1 OVFT
*
*timer1 = 1 / (16M / 1024 / 5)320 micro second  3125Hz
*timer3 = 1 / (16M / 1024 / 3125)200 milli second  5Hz
*timer4 = 1 / (16M / 1024 / 3906)250 milli second  4Hz
*timer5 = 1 / (16M / 1024 / 781)50 milli second   20Hz
*
*1
*   1 2 3 4
* 1          
* 2        
* 3        
* 4        
*
*2
*   1 2 3 4
* 1        
* 2        
* 3        
* 4        
*
* 3
*   1 2 3 4
* 1        
* 2        
* 3        
* 4        
*
*
*
*/

//經過一番精巧的騷操作 結果就出來了 原理管他的 反正能用就好
#include <LinkedList.h>             //引入鏈結串列函式庫(https://github.com/ivanseidel/LinkedList/issues/39)
#include <LiquidCrystal_I2C.h>  //引入LCD函式庫
#include <EEPROM.h>                 //引入EEPROM函式庫
#include <Wire.h>

#define led_array_length 5              //led陣列長度
#define botton1_pin 8                   //按鈕1腳位
#define botton2_pin 9                   //按鈕2腳位
#define score_highest_easy_eeprom_address 0  //最高分在eeprom的位子
#define score_highest_normal_eeprom_address 1  //最高分在eeprom的位子
#define score_highest_hard_eeprom_address 2  //最高分在eeprom的位子
#define interrupt_pin 19                //外部中斷腳位



LiquidCrystal_I2C lcd(0x3F, 16, 2);       //宣告LCD物件
LinkedList<byte> snake_coordinate_x;      //貪食蛇座標
LinkedList<byte> snake_coordinate_y;      //貪食蛇座標
LinkedList<byte> snake_coordinate_z;      //貪食蛇座標


byte joystick_x = A6;
byte joystick_y = A7;

byte apple_coordinate[3];                           //貪食蛇座標
byte snake_move_direction;                          //蛇最終要移動的方向
byte joystick_input;                                //搖桿輸入
byte print_data = 0;                                //LED陣列顯示資料
byte led_layer = 0;                                 //LED掃描層數
byte continued = 0;                                 //遊戲是否繼續
byte score;                                         //分數
byte apple_flash = 1;                               //蘋果是否亮起
byte score_highest;
byte led_port[led_array_length];                //PORTF,PORTB,PORTL,PORTC,PORTA
byte common_port;
int step_delay_array[3] = {1200, 1000, 600};
int step_delay_time = 800;
byte led_count = 0;


int joystick_x_level;                               //設定搖桿0準位
int joystick_y_level;

const byte tccr0 = TCCR0B;
bool timer_enable = 1;

byte matrix[4][led_array_length][led_array_length][led_array_length] = //顯示陣列(x,y,z)
{
    {//遊玩的畫面
        {{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0}},
        {{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0}},
        {{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0}},
        {{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0}},
        {{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0}}
    },
    {//倒數 1
        {{0, 1, 1, 1, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0}},
        {{0, 0, 1, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0}},
        {{0, 0, 1, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0}},
        {{0, 1, 1, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0}},
        {{0, 0, 1, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0}}
    },
    {//倒數 2
        {{0, 1, 1, 1, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0}},
        {{0, 1, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0}},
        {{0, 1, 1, 1, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0}},
        {{0, 0, 0, 1, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0}},
        {{0, 1, 1, 1, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0}}
    },
    {//倒數 3
        {{0, 1, 1, 1, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0}},
        {{0, 0, 0, 1, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0}},
        {{0, 1, 1, 1, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0}},
        {{0, 0, 0, 1, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0}},
        {{0, 1, 1, 1, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0}}
    }
};

byte dir[6][5] = 
{
    {5, 4, 2, 3, 0},
    {4, 5, 2, 3, 1},
    {0, 1, 5, 4, 2},
    {0, 1, 4, 5, 3},
    {0, 1, 2, 3, 4},
    {0, 1, 2, 3, 5}
};

void display_array_reset();     //重設led顯示陣列
void pinmod_set();              //設定接腳模式
void lcd_set();                 //設定LCD
void timer_set();               //設定timer
void snake_reset();             //貪食蛇重製座標

void apple_create();            //生成蘋果
void apple_to_matrix();         //將蘋果加入顯示陣列中

void joystick_direction();      //取得搖桿方向
void snake_direction();         //判定貪食蛇移動方向

void snake_move();              //貪食蛇移動
void snake_hit(byte x, byte y, byte z); //蛇是否有碰撞
void snake_to_matrix();         //將貪食蛇陣列內容加入至顯示陣列中
void reflash_score(byte delta); //刷新分數
void count_down();              //倒數321
void write_scan_register();       //將變數寫入暫存器中
void register_clear();          //清空站存器

byte count_digit(byte num);     //取得數值位數



void lcd_display();//lcd顯示



void debug_input_led_array();   //輸入陣列資料
void debug_print_led_array();   //顯示顯示陣列內容
void debug_timer_change();      //改變timer啟用狀態

void setup()
{
    Serial.begin(115200);       //設定Serial鮑率
    lcd_set();                  //設定lcd
    pinmod_set();               //設定接腳模式
    timer_set();                //設定timer


    joystick_x_level = analogRead(joystick_x);
    joystick_y_level = analogRead(joystick_y);
    
    //debug_input_led_array();
    //debug_print_led_array();
    //display_array_reset();
    attachInterrupt(digitalPinToInterrupt(interrupt_pin), debug_timer_change, RISING);       
    
    randomSeed(analogRead(A7)); //設定隨機亂數種子碼
}

void loop() 
{
    snake_move_direction = 0;
    int jx, jy;
    score = 0;
    
    //Serial.println("open\n\n");
    lcd_display(0);             //LCD顯示開始畫面

    joystick_input = 4;
    step_delay_time = step_delay_array[1];
    
    while(digitalRead(botton1_pin) == 0)
    {
        joystick_direction();
        
        if(joystick_input == 0 || joystick_input == 1)
        {
            step_delay_time = step_delay_array[1];
            
            lcd.setCursor(0, 0);
            lcd.print("  normal level  ");
        }
        else if(joystick_input == 3)
        {
            step_delay_time = step_delay_array[0];
            
            lcd.setCursor(0, 0);
            lcd.print("   easy level   ");
        }
        else if(joystick_input == 2)
        {
            step_delay_time = step_delay_array[2];
            
            lcd.setCursor(0, 0);
            lcd.print("   hard level   ");
        }
            
        lcd.setCursor(0, 1);
        lcd.print("press the joycon");

        
        unsigned long tmp_delay = millis();
        while(millis() - tmp_delay < 1){}
    }   //等待按鈕按下
    Serial.println(step_delay_time);
    if(step_delay_time == 600)
    {
        score_highest = EEPROM.read(score_highest_hard_eeprom_address);  //把最高紀錄從EEPROM中取出
    }
    if(step_delay_time == 1000)
    {
        score_highest = EEPROM.read(score_highest_normal_eeprom_address);  //把最高紀錄從EEPROM中取出
    }
    if(step_delay_time == 1200)
    {
        score_highest = EEPROM.read(score_highest_easy_eeprom_address);  //把最高紀錄從EEPROM中取出
    }
    
    continued = 1;
        
    
    lcd_display(1);           //顯示遊玩畫面
    display_array_reset();
    
    snake_reset();              //重設貪食蛇
    apple_create();             //生成第一顆蘋果
    snake_to_matrix();
    
    count_down();               //倒數321
    //Serial.println("begin2");
    

    while(continued)
    {
        //Serial.println("playing1");
        display_array_reset();
        snake_to_matrix();
        apple_to_matrix();
        unsigned long joystick_delay = millis();      //設定時間戳記
        //Serial.println("playing2");
        while(millis() - joystick_delay < step_delay_time)    //若時間還沒過0.5s
        {
            joystick_direction();   //取得搖桿方向
            unsigned long tmp_delay = millis();
            while(millis() - tmp_delay < 1){}
        }
        debug_print_led_array();
        //Serial.println("move");
        snake_direction();          //判定貪食蛇移動方向
        //Serial.println(snake_move_direction);
        
        snake_move();               //貪食蛇移動
        
        //Serial.println("playingend\n\n");
    }
    display_array_reset();
    continued = 0;
    //Serial.print("finish1");
    if(score > score_highest)       //當分數比最高分高
    {
        lcd_display(2);             //顯示結束畫面(贏)
        Serial.println("finish display1");
        if(step_delay_time == 600)
        {
            EEPROM.update(score_highest_hard_eeprom_address, score);  //把最高紀錄從EEPROM中取出
        }
        if(step_delay_time == 1000)
        {
            EEPROM.update(score_highest_normal_eeprom_address, score);  //把最高紀錄從EEPROM中取出
        }
        if(step_delay_time == 1200)
        {
            EEPROM.update(score_highest_easy_eeprom_address, score);  //把最高紀錄從EEPROM中取出
        }
    }
    else
    {
        lcd_display(3);             //顯示結束畫面(一班)
        //Serial.println("finish display2");
    }
    
    Serial.println("finish\n\n");
    
    while(digitalRead(botton1_pin) == 0){}   //等待按鈕按下
    unsigned long time_last = millis();
    while(millis() - time_last < 500){}
    Serial.println("end");
}

void display_array_reset()//重設led顯示陣列
{
    for(int i = 0; i < led_array_length; i++)
    {
        for(int j = 0; j < led_array_length; j++)
        {
            for(int k = 0; k < led_array_length; k++)
            {
                matrix[0][i][j][k] = 0;
            }
        }
    }
}

void pinmod_set()//設定接腳模式
{
    DDRF |= 0xFF;   //設定掃描
    DDRB |= 0xFF;   //設定led接腳
    DDRL |= 0xFF;   //設定led接腳
    DDRC |= 0xFF;   //設定led接腳
    DDRA |= 0xFF;   //設定led接腳
    DDRK |= 0xFF;   //設定led接腳

    PORTF = 0xFF;
    PORTB = 0xFF;
    PORTL = 0xFF;
    PORTC = 0xFF;
    PORTA = 0xFF;
    PORTK = 0xFF;
    
    pinMode(botton1_pin, INPUT);    //設定按鈕一接腳為輸入
    pinMode(botton2_pin, INPUT);    //設定按鈕一接腳為輸入
    pinMode(joystick_x, INPUT);     //設定搖桿X軸接腳為輸入
    pinMode(joystick_y, INPUT);     //設定搖桿Y軸腳為輸入
}

void lcd_set()//設定LCD
{
    lcd.init();
    lcd.setBacklight(255);  //設定lcd亮度
    lcd.noCursor();         //不顯示光標
    lcd.clear();            //清除lcd
}

void timer_set()//設定timer
{
    TCCR1A = 0x00;          //啟用TIMER1計時中斷
    TCCR1B |= _BV(CS12);    //設定除頻1024
    TCCR1B &= ~_BV(CS11);
    TCCR1B |= _BV(CS10);
    TIMSK1 |= _BV(TOIE1);   //啟用一位中斷
    TCNT1 = -5;             //5周中斷一次

    TCCR3A = 0x00;          //啟用TIMER1計時中斷
    TCCR3B |= _BV(CS12);    //設定除頻1024
    TCCR3B &= ~_BV(CS11);
    TCCR3B |= _BV(CS10);
    TIMSK3 |= _BV(TOIE1);   //啟用一位中斷
    TCNT3 = -3125;            //5周中斷一次
    
    TCCR4A = 0x00;          //啟用TIMER1計時中斷
    TCCR4B |= _BV(CS12);    //設定除頻1024
    TCCR4B &= ~_BV(CS11);
    TCCR4B |= _BV(CS10);
    TIMSK4 |= _BV(TOIE1);   //啟用一位中斷
    TCNT4 = -1562;            //5周中斷一次

    TCCR5A = 0x00;          //啟用TIMER1計時中斷
    TCCR5B |= _BV(CS12);    //設定除頻1024
    TCCR5B &= ~_BV(CS11);
    TCCR5B |= _BV(CS10);
    TIMSK5 |= _BV(TOIE1);   //啟用一位中斷
    TCNT5 = -1562;            //5周中斷一次
}

void snake_reset()              //貪食蛇重製座標
{
    snake_coordinate_x.clear();       //清除貪食蛇座標陣列
    snake_coordinate_y.clear();       //清除貪食蛇座標陣列
    snake_coordinate_z.clear();       //清除貪食蛇座標陣列

    LinkedList<byte> tmp;           //新增一個暫時的陣列
    tmp.add(2); //設定X軸座標
    tmp.add(0); //設定Y軸座標
    tmp.add(4); //設定Z軸座標


    for(int i = 0; i < 3; i++)
    {
        snake_coordinate_x.unshift(tmp[0]);  //將臨時陣列猜入座標陣列中
        snake_coordinate_y.unshift(tmp[1]);  //將臨時陣列猜入座標陣列中
        snake_coordinate_z.unshift(tmp[2]);  //將臨時陣列猜入座標陣列中
        tmp[1] += 1;    //將y軸座標+1
    }
}

void apple_create()             //生成蘋果
{
    byte accept = 0;    //重複
    byte coordinate[3]; //臨時蘋果座標陣列
    do
    {
        accept = 0; //重複0個軸座標

        for(int i = 0; i < 3; i++)
        {
            coordinate[i] = random(led_array_length);//座標隨機(0~3)
        }
        
        for(int i = 0; i < snake_coordinate_x.size(); i++)
        {
            if((snake_coordinate_x[i] == coordinate[0])&&
               (snake_coordinate_y[i] == coordinate[1])&&
               (snake_coordinate_z[i] == coordinate[2]))//如果x軸座標一樣
            {
                accept = 1;
            }
        }
    } while(accept);//如果重複，重隨機一個點
    for(int i = 0; i < 3; i++)
    {
        apple_coordinate[i] = coordinate[i];//將臨時座標放入蘋果座標陣列
    }
    apple_to_matrix();
}

void apple_to_matrix()
{
    matrix[0][apple_coordinate[2]][apple_coordinate[1]][apple_coordinate[0]] = 2;
}

void joystick_direction()       //取得搖桿方向*****(讀取程式)    前後左右 0123
{
    int x, y;
    x = analogRead(joystick_x);
    y = analogRead(joystick_y);
    //Serial.println(joystick_x_level);
    //Serial.println(joystick_y_level);

    if(x - joystick_x_level< -300)
    {
        joystick_input = 0;
    }
    else if(x - joystick_x_level > 300)
    {
        joystick_input = 1;
    }
    else if(y - joystick_y_level > 300)
    {
        joystick_input = 2;
    }
    else if(y - joystick_y_level < -300)
    {
        joystick_input = 3;
    }
    else
    {
        joystick_input = 4;
    }
}

void snake_direction()          //判定貪食蛇移動方向   前後左右上下 012345
{
    snake_move_direction = dir[snake_move_direction][joystick_input];
}


void snake_move()               //貪食蛇移動
{
    LinkedList<int> tmp;//(xyz)
    tmp.clear();
    tmp.add(snake_coordinate_x[0]);
    tmp.add(snake_coordinate_y[0]);
    tmp.add(snake_coordinate_z[0]);
    switch(snake_move_direction)
    {
        case 0://前
            tmp[1] += 1;
            break;
        case 1://後
            tmp[1] -= 1;
            break;
        case 2://左
            tmp[0] -= 1;
            break;
        case 3://右
            tmp[0] += 1;
            break;
        case 4://上
            tmp[2] += 1;
            break;
        case 5://下
            tmp[2] -= 1;
            break;
    }
    for(int i = 0; i < 3; i++)
    {
        tmp[i] = (tmp[i] + led_array_length) % led_array_length;
    }
    
    snake_coordinate_x.unshift(tmp[0]);
    snake_coordinate_y.unshift(tmp[1]);
    snake_coordinate_z.unshift(tmp[2]);
    snake_hit(tmp[0], tmp[1], tmp[2]);//蛇是否有碰撞
    
}

void snake_hit(byte x, byte y, byte z)//蛇是否有碰撞
{
    if((apple_coordinate[0] == x) && (apple_coordinate[1] == y) && (apple_coordinate[2] == z))//當頭撞到蘋果
    {
        score += 1;
        lcd_display(1);
        apple_create();
    }
    else//如果沒碰到蘋果
    {
        snake_coordinate_x.pop();
        snake_coordinate_y.pop();
        snake_coordinate_z.pop();
    }
    for(int i = 1; i < snake_coordinate_x.size(); i++)//當頭撞到身體
    {
        if( (snake_coordinate_x[i] == snake_coordinate_x[0])&&
            (snake_coordinate_y[i] == snake_coordinate_y[0])&& 
            (snake_coordinate_z[i] == snake_coordinate_z[0])
          )
        {
            continued = 0;
        }
    }
}

void snake_to_matrix()
{
    for(int i = 0; i <snake_coordinate_x.size(); i++)
    {
        matrix[0][snake_coordinate_z[i]][snake_coordinate_y[i]][snake_coordinate_x[i]] = 1;
    }
}

void reflash_score(byte delta)  //刷新分數
{
    score += delta;
    lcd_display(1);
}

void count_down()               //倒數321
{
    for(int i = 3; i >= 0; i--)
    {
        print_data = i;
        unsigned long count_down_delay = millis();
        while((count_down_delay + 1000) > millis()){}
        Serial.println(i);
    }
    print_data = 0;
}

void write_scan_register()      //將變數寫入暫存器中
{
    PORTF = led_port[0];
    PORTB = led_port[1];
    PORTL = led_port[2];
    PORTC = led_port[3];
    PORTA = led_port[4];
}

void register_clear()           //清除LED顯示內容
{
    PORTK &= 0x00;
    PORTF &= 0x00;
    PORTB &= 0x00;
    PORTL &= 0x00;
    PORTC &= 0x00;
    PORTA &= 0x00;
}

byte count_digit(byte num)      //取得數值位數
{
    byte count = 0;
    if(num == 0)
    {
        return 1;
    }
    else
    {
        while(num != 0)
        {
            num /= 10;
            count += 1;
        }
        return count;
    }
}

void lcd_display(byte mod)      //lcd顯示
{
    lcd.clear();
    if(mod == 0)//begin
    {
        lcd.setCursor(0, 0);
        lcd.print("5x5x5 snake game");
        
        lcd.setCursor(0, 1);
        lcd.print("press the joycon");
    }
    else if(mod == 1)//playing
    {
        lcd.setCursor(0, 0);
        lcd.print("playing");
    
        lcd.setCursor(0, 1);
        lcd.print("score:");
        Serial.println(count_digit(score));
        for(int i = 0; i < 10 - count_digit(score); i++)
        {
            lcd.print(' ');
        }
        if(score)
        {
            lcd.print(score);
        }
        else
        {
            lcd.print("0");
        }
    }
    else if(mod == 2)//finish_win
    {
        lcd.setCursor(0, 0);
        lcd.print("new record!!!!");

        lcd.setCursor(0, 1);
        lcd.print("score:");
        for(int i = 0; i < 10 - count_digit(score); i++)
        {
            lcd.print(' ');
        }
        lcd.print(score);
    }
    else if(mod = 3)//finish_lose
    {
        lcd.setCursor(0, 0);
        lcd.print("score:");
        
        for(int i = 0; i < 10 - count_digit(score); i++)
        {
            lcd.print(' ');
        }
        lcd.print(score);
        
        lcd.setCursor(0, 1);
        lcd.print("high record:");
        for(int i = 0; i < 4 - count_digit(score_highest); i++)
        {
            lcd.print(' ');
        }
        if(score_highest)
        {
            lcd.print(score_highest);
        }
        else
        {
            lcd.print('0');
        }
        
        //Serial.print("score_highest:");
        //Serial.println(score_highest);
    }
}

ISR(TIMER1_OVF_vect)            //掃描中斷 320micro
{//掃描LED三維陣列
    TCNT1 = -52;//周中斷一次
    
    for(int j = 0;j < led_array_length; j++)
    {
        for(int i = 0; i < led_array_length; i++)//顯示
        {
            if(matrix[print_data][led_layer][j][i] == 1)//當顯示陣列內容為蛇的身體時
            {
                led_port[j] &= ~_BV(i);
            }
            /*
            else if(matrix[print_data][led_layer][j][i] == 2)//當顯示陣列內容為蘋果時
            {
                if(apple_flash)
                {
                    led_port[j] &= ~_BV(i);
                }
            }
            */
            else
            {
                led_port[j] |= _BV(i);
            }
        }
    }
    
    PORTF = led_port[0];
    PORTB = led_port[1];
    PORTL = led_port[2];
    PORTC = led_port[3];
    PORTA = led_port[4];

    PORTK = 0x00;
    PORTK = _BV(led_layer);

    led_layer = (led_layer + 1) % led_array_length; //掃描計數+1
}

ISR(TIMER3_OVF_vect)            //蘋果閃爍 掃描LED三維陣列
{
    TCNT3 = -3906;//50周中斷一次
    /*
    apple_flash = (apple_flash + 1) % 2;
    */
    if(continued)
    {
        if(matrix[0][apple_coordinate[2]][apple_coordinate[1]][apple_coordinate[0]] == 1)
        {
            matrix[0][apple_coordinate[2]][apple_coordinate[1]][apple_coordinate[0]] = 0;
        }
        else
        {
            matrix[0][apple_coordinate[2]][apple_coordinate[1]][apple_coordinate[0]] = 1;
        }
    }
}

ISR(TIMER4_OVF_vect)        //蛇頭閃爍
{
    TCNT4 = -764;
    if(continued)
    {
        if(matrix[0][snake_coordinate_z[0]][snake_coordinate_y[0]][snake_coordinate_x[0]] == 1)
        {
            matrix[0][snake_coordinate_z[0]][snake_coordinate_y[0]][snake_coordinate_x[0]] = 0;
        }
        else
        {
            matrix[0][snake_coordinate_z[0]][snake_coordinate_y[0]][snake_coordinate_x[0]] = 1;
        }
    }
}

void debug_input_led_array()
{/*
    for(int i = 0; i < led_array_length; i++)
    {
        while(Serial.available() == 0);

        String a;
        a = Serial.readStringUntil("\0");
        Serial.println(a);
        for(int j = 0; j < led_array_length; j++)
        {
            for(int k = 0; k < led_array_length; k++)
            {
                matrix[0][i][j][k] =  a[j * 5 + k] - 48;
            }
        }
    }*/
}

void debug_print_led_array()
{
    for(int i = 0; i < led_array_length; i++)
    {
        for(int j = 0; j < led_array_length; j++)
        {
            for(int k = 0; k < led_array_length; k++)
            {
                Serial.print(matrix[0][i][j][k]);
                Serial.print(" ");
            }
            Serial.print(" ");
        }
        Serial.println();
    }
    Serial.println();
}


void debug_timer_change()   //改變timer啟用狀態
{/*
    if(timer_enable)
    {
        TCCR0B = 0x00;
    } 
    else
    {
        TCCR0B = tccr0;
    }
    timer_enable ^= 0x01;*/
}

ISR(TIMER5_OVF_vect)        //搖桿輸入
{
    TCNT5 = -130;
    if(continued == 0)
    {
        for(int i = 0; i < 2; i++)
        {
            byte tmp[3];
            for(int i = 0; i < 3; i++)
            {
                tmp[i] = random(led_array_length);//座標隨機(0~3)
            }
            
            if(matrix[0][tmp[2]][tmp[1]][tmp[0]] == 0 && led_count < 20)
            {
                matrix[0][tmp[2]][tmp[1]][tmp[0]] = 1;
                led_count += 1;
            }
            else if(matrix[0][tmp[2]][tmp[1]][tmp[0]] == 1)
            {
                matrix[0][tmp[2]][tmp[1]][tmp[0]] = 0;
                led_count -= 1;
            }
        }
        
        Serial.println("r");
    }
}
