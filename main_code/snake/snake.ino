//電子112甲 專題 3D立體貪食蛇
//LED三維矩陣、搖桿、按鈕、LCD
//((4*3)*16) (2+1)  (1) (4)  元件使用腳位數
/*
begin
0123456789abcdef
3-D snake game
botton-A to play

playing
0123456789abcdef
high score: xxx
           1234
score:      xxx
      123456789

win
0123456789abcdef
new record!!!!  

score:       xxx
      0123456789


lose
0123456789abcdef
score:       xxx
      0123456789
*/

//經過一番精巧的騷操作 結果就出來了 原理管他的 反正能用就好
#include <LinkedList.h>             //引入鏈結串列函式庫(https://github.com/ivanseidel/LinkedList/issues/39)
#include <LiquidCrystal_PCF8574.h>  //引入LCD函式庫
#include <EEPROM.h>                 //引入EEPROM函式庫

#define led_array_length 4              //led陣列長度
#define botton1_pin 8                   //按鈕1腳位
#define botton2_pin 9                   //按鈕2腳位
#define joystick_x 5                    //搖桿x軸腳位
#define joystick_y 6                    //搖桿y軸腳位
#define score_highest_eeprom_address 0  //最高分在eeprom的位子

LiquidCrystal_PCF8574 lcd(0x27);        //宣告LCD物件
LinkedList<LinkedList<byte>> snake_coordinate;      //貪食蛇座標
byte apple_coordinate[3];                           //貪食蛇座標
byte snake_move_direction;                          //蛇最終要移動的方向
byte joystick_input;                                //搖桿輸入
byte print_data = 0;                                //LED陣列顯示資料
byte led_layer = 0;                                 //LED掃描層數
byte continued = 1;                                 //遊戲是否繼續
byte score;                                         //分數
byte apple_flash = 1;                               //蘋果是否亮起
byte score_highest;
byte led_port[led_array_length];                //PORTF,PORTB,PORTL,PORTC,PORTA
byte common_port;

byte matrix[4][led_array_length][led_array_length][led_array_length] = //顯示陣列(x,y,z)
{
    {//LED三維陣列顯示
        {{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0}},//PORTA0~3
        {{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0}},//PORTA4~7
        {{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0}},//PORTB0~3
        {{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0}} //PORTB4~7
    },
    {//LED三維陣列顯示1
        {{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0}},//PORTA0~3
        {{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0}},//PORTA4~7
        {{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0}},//PORTB0~3
        {{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0}} //PORTB4~7
    },
    {//LED三維陣列顯示2
        {{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0}},//PORTA0~3
        {{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0}},//PORTA4~7
        {{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0}},//PORTB0~3
        {{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0}} //PORTB4~7
    },
    {//LED三維陣列顯示3
        {{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0}},//PORTA0~3
        {{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0}},//PORTA4~7
        {{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0}},//PORTB0~3
        {{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0},{0, 0, 0, 0}} //PORTB4~7
    }
};

byte dir[6][4] = 
{
    {4,5,2,3},
    {4,5,2,3},
    {0,1,5,4},
    {0,1,4,5},
    {0,1,2,3},
    {0,1,2,3}
};

void display_array_reset();     //重設led顯示陣列
void pinmod_set();              //設定接腳模式
void lcd_set();                 //設定LCD
void timer_set();               //設定timer
void snake_reset();             //貪食蛇重製座標

void create_apple();            //生成蘋果*****

void print_led_dot(byte x, byte y, byte z, byte output_data);   //將要顯示資料寫入矩陣中
void joystick_direction();      //取得搖桿方向
void snake_direction();         //判定貪食蛇移動方向

void snake_move();              //貪食蛇移動
void snake_hit(byte x, byte y, byte z); //蛇是否有碰撞
void reflash_score(byte delta); //刷新分數
void count_down();              //倒數321
void variable_register();       //將變數寫入暫存器中
void register_clear();          //清空站存器

void led_reflash();             //刷新LED顯示內容

byte count_digit(byte num);     //取得數值位數

byte EEPROM_read(int address);  //取得eeprom資料
void EEPROM_write(int address, byte value); //將資料寫入eeprom

void lcd_display();//lcd顯示

void debug_input_led_array();   //輸入陣列資料
void debug_print_led_array();   //顯示顯示陣列內容

void setup() 
{
    Serial.begin(115200);       //設定Serial鮑率
    lcd_set();                  //設定lcd
    pinmod_set();               //設定接腳模式
    timer_set();                //設定timer

    debug_input_led_array();
    debug_print_led_array();

    score_highest = EEPROM_read(score_highest_eeprom_address);  //把最高紀錄從EEPROM中取出

    randomSeed(analogRead(A8)); //設定隨機亂數種子碼
}

void loop() 
{
    lcd_display(0);             //LCD顯示開始畫面
    score_highest = EEPROM_read(score_highest_eeprom_address);  //把最高紀錄從EEPROM中取出

    while(digitalRead(botton1_pin) == 0);   //等待按鈕按下
    lcd_display(1);             //顯示遊玩畫面
    count_down();               //倒數321
    snake_reset();              //重設貪食蛇
    display_array_reset();      //重設led顯示陣列
    while(continued)            //當遊戲持續
    {
        int joystick_delay = millis();      //設定時間戳記
        while((joystick_delay + 500) > millis())    //若時間還沒過0.5s
        {
            joystick_direction();   //取得搖桿方向
        }
        snake_direction();          //判定貪食蛇移動方向
        snake_move();               //貪食蛇移動
    }
    
    if(score > score_highest)       //當分數比最高分高
    {
        lcd_display(2);             //顯示結束畫面(贏)
        EEPROM_write(score_highest_eeprom_address, score);//將分數寫入EEPROM
    }
    else
    {
        lcd_display(3);             //顯示結束畫面(一班)
    }

    while(digitalRead(botton1_pin) == 0);//等待按鈕按下
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
    pinMode(botton1_pin, 0);    //設定按鈕一接腳為輸入
    pinMode(botton2_pin, 0);    //設定按鈕一接腳為輸入
    pinMode(joystick_x, 0);     //設定搖桿X軸接腳為輸入
    pinMode(joystick_y, 0);     //設定搖桿Y軸腳為輸入

    DDRA |= 0xFF;   //設定掃描
    DDRB |= 0xFF;   //設定led接腳
    DDRC |= 0xFF;   //設定led接腳

    register_clear();
}

void lcd_set()//設定LCD
{
    lcd.begin(16, 2);       //設定lcd行、列
    lcd.setBacklight(255);  //設定lcd亮度
    lcd.noCursor();         //不顯示光標
    lcd.clear();            //清除lcd
}

void timer_set()//設定timer
{
    TCCR3A = 0x00;          //啟用TIMER1計時中斷
    TCCR3B |= _BV(CS12);    //設定除頻1024
    TCCR3B &= ~_BV(CS11);
    TCCR3B |= _BV(CS10);
    TIMSK3 |= _BV(TOIE1);   //啟用一位中斷
    TCNT3 = -5;             //5周中斷一次

    TCCR4A = 0x00;          //啟用TIMER1計時中斷
    TCCR4B |= _BV(CS12);    //設定除頻1024
    TCCR4B &= ~_BV(CS11);
    TCCR4B |= _BV(CS10);
    TIMSK4 |= _BV(TOIE1);   //啟用一位中斷
    TCNT4 = -50;            //5周中斷一次
}

void snake_reset()//貪食蛇重製座標
{
    snake_coordinate.clear();       //清除貪食蛇座標陣列
    LinkedList<byte> tmp;           //新增一個暫時的陣列
    tmp.add(4); //設定X軸座標
    tmp.add(2); //設定Y軸座標
    tmp.add(0); //設定Z軸座標
    for(int i = 0; i < 3; i++)      //重製蛇每節的座標
    {
        snake_coordinate.add(tmp);  //將臨時陣列猜入座標陣列中
        tmp[2] += 1;    //將y軸座標+1
    }
}

void create_apple()//生成蘋果
{
    byte accept = 0;    //重複 (=3)
    byte coordinate[3]; //臨時蘋果座標陣列
    do
    {
        accept = 0; //重複0個軸座標

        for(int i = 0; i < 3; i++)
        {
            coordinate[i] = random(4);//座標隨機(0~3)
        }
        
        for(int i = 0; i < snake_coordinate.size(); i++)
        {
            for(int j = 0; j < 3; j++)
            {
                if(snake_coordinate[i][j] == coordinate[j])//如果軸座標一樣
                {
                    accept += 1;//多重複一個軸座標
                }
            }
        }
    } while(accept == 3);//如果重複，重隨機一個點
    for(int i = 0; i < 3; i++)
    {
        apple_coordinate[i] = coordinate[i];//將臨時座標放入蘋果座標陣列
    }
    print_led_dot(apple_coordinate[0], apple_coordinate[1], apple_coordinate[2], -1);//將要蘋果座標寫入矩陣中
}

void print_led_dot(byte x, byte y, byte z, byte output_data)//將要顯示資料寫入矩陣中
{
    matrix[0][x][y][z] = output_data;
}

void joystick_direction()//取得搖桿方向*****(讀取程式)    前後左右 0123
{
    int x, y;
    x = analogRead(joystick_x) - 512;
    y = analogRead(joystick_y) - 512;

    
}

void snake_direction()//判定貪食蛇移動方向   前後左右上下 012345
{
    byte snake_tmp = joystick_input;
    snake_move_direction = dir[snake_move_direction][snake_tmp];
}

void snake_move()//貪食蛇移動
{
    LinkedList<byte> tmp;//(xyz)
    for(int i = 0; i < 3; i++)
    {
        tmp.add(snake_coordinate[0][i]);
    }
    switch(snake_move_direction)
    {
        case1://上
            tmp[2] += 1;
            break;
        case2://下
            tmp[2] -= 1;
            break;
        case3://左
            tmp[1] += 1;
            break;
        case4://右
            tmp[1] -= 1;
            break;
        case5://前
            tmp[0] += 1;
            break;
        case6://後
            tmp[0] -= 1;
            break;
    }
    snake_hit(tmp[0], tmp[1], tmp[2]);//蛇是否有碰撞
    snake_coordinate.add(0,tmp);
    for(int i = 0; i < snake_coordinate.size(); i++)
    {
        print_led_dot(snake_coordinate[i][0],snake_coordinate[i][1], snake_coordinate[i][2], 1);//將要蛇座標寫入矩陣中
    }
}

void snake_hit(byte x, byte y, byte z)//蛇是否有碰撞
{

    for(int i = 0; i < snake_coordinate.size(); i++)//當頭超出範圍
    {
        for(int j = 0; j < 3; j++)/////
        {
            if((snake_coordinate[i][j] > (led_array_length - 1))||(snake_coordinate[i][j] < 0))
            {
                continued = 0;
            }
        }
    }
    for(int i = 0; i < snake_coordinate.size(); i++)//當頭撞到身體
    {
        if((snake_coordinate[i][0] == x) && (snake_coordinate[i][1] == y) && (snake_coordinate[i][2] == z))
        {
            continued = 0;
        }
    }
    if(continued && (apple_coordinate[0] == x) && (apple_coordinate[1] == y) && (apple_coordinate[2] == z))//當頭撞到蘋果
    {
        score += 1;
        lcd_display(1);
    }
    else//如果沒碰到蘋果
    {
        snake_coordinate.pop();
    }
}

void reflash_score(byte delta)//刷新分數
{
    score += delta;
    lcd_display(1);
}

void count_down()//倒數321
{
    int count_down_delay = millis();
    for(int i = 0; i < 3; i++)
    {
        while((count_down_delay + 1000) < millis())
        {
            print_data = 3 - i;
        }
    }
    print_data = 0;
}

void variable_register()//將變數寫入暫存器中
{
    PORTF = led_port[0];
    PORTB = led_port[1];
    PORTL = led_port[2];
    PORTC = led_port[3];
    PORTA = led_port[4];
}

void register_clear()//清除LED顯示內容
{
    PORTF &= 0x00;
    PORTB &= 0x00;
    PORTL &= 0x00;
    PORTC &= 0x00;
    PORTA &= 0x00;
}

void led_reflash()//刷新LED顯示內容
{
    if(led_array_length < 6)
    {
        for(int j = 0;j < led_array_length; j++)
        {
            for(int i = 0; i < led_array_length; i++)//顯示
            {
                if(matrix[print_data][led_layer][j][i] == byte(1))//當顯示陣列內容為蛇的身體時
                {
                    led_port[j] |= _BV(i);
                }
                else if(matrix[print_data][led_layer][j][i] == byte(-1))//當顯示陣列內容為蘋果時
                {
                    if(apple_flash == 1)
                    {
                        led_port[j] |= _BV(i);
                    }
                }
                else
                {
                    led_port[j] &= ~_BV(i);
                }
            }
        }
    }
    else
    {
        int count_tmp = 0;
        for(int i = 0; i < 36; i++)
        {
            if(matrix[print_data][led_layer][i / 6][i % 6] == byte(1))//當顯示陣列內容為蛇的身體時
            {
                led_port[i / 8] |= _BV((i % 8) + 1);
            }
            else if(matrix[print_data][led_layer][count_tmp / 6][count_tmp % 6] == byte(-1))//當顯示陣列內容為蘋果時
            {
                if(apple_flash == 1)
                {
                    led_port[i / 8] |= _BV((i % 8) + 1);
                }
            }
            else
            {
                led_port[i / 8] |= _BV((i % 8) + 1);
            }
        }
    }
    variable_register();
}

byte count_digit(byte num)//取得數值位數
{
    byte count = 0;
    while(num != 0)
    {
        num %= 10;
        count += 1;
    }
    return count;
}

byte EEPROM_read(int address)//取得eeprom資料
{
    return EEPROM.read(address);
}

void EEPROM_write(int address, byte value)//將資料寫入eeprom
{
    EEPROM.write(address, value);
}

void lcd_display(byte mod)//lcd顯示
{
    lcd.clear();
    if(mod == 0)//begin
    {
        lcd.setCursor(0, 0);
        lcd.print("3-D snake game");
        
        lcd.setCursor(0, 1);
        lcd.print("botton-A to play");
    }
    else if(mod == 1)//playing
    {
        lcd.setCursor(0, 0);
        lcd.print("high score:");
        for(int i = 0; i < 4 - count_digit(score_highest); i++)
        {
            lcd.print(' ');
        }
        lcd.print(score_highest);

        lcd.setCursor(0, 1);
        lcd.print("score: ");
        for(int i = 0; i < 9 - count_digit(score); i++)
        {
            lcd.print(' ');
        }
        lcd.print(score);
    }
    else if(mod == 2)//finish_win
    {
        lcd.setCursor(0, 0);
        lcd.print("new record!!!!");

        lcd.setCursor(0, 1);
        lcd.print("score:");
        for(int i = 0; i < 9 - count_digit(score); i++)
        {
            lcd.print(' ');
        }
        lcd.print(score);
    }
    else if(mod = 3)//finish_lose
    {
        lcd.setCursor(0, 0);
        lcd.print("score:");
        
        lcd.setCursor(0, 1);
        for(int i = 0; i < 9 - count_digit(score); i++)
        {
            lcd.print(' ');
        }
        lcd.print(score);
    }
}

ISR(TIMER3_OVF_vect)//掃描中斷
{//掃描LED三維陣列
    TCNT3 = -5;//5周中斷一次
    led_reflash();//刷新LED顯示內容
    PORTK |= _BV(led_layer);
    led_layer = (led_layer + 1) % led_array_length; //掃描計數+1
}

ISR(TIMER4_OVF_vect)//蘋果閃爍 掃描LED三維陣列
{
    TCNT4 = -50;//50周中斷一次
    apple_flash = (apple_flash + 1) % 2;
}

void debug_input_led_array()
{
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
    }
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
}
