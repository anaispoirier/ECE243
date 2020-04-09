#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

#define LAUNCH_ROW 11
#define LAUNCH_COL 12

typedef struct Pair {
	int row;
	int col;
} Pair;

typedef struct Bubble { 
   int xc;
   int yc;
   int radius;
   int colour;
   bool visible;
   Pair up;
   Pair down;
   Pair right;
   Pair left;
} Bubble;

#define PI 3.14159265359
#define RADIANS_TO_DEGREES 57.2957795131
#define DEGREE_TO_RADIANS 0.01745329251
typedef struct LaunchBubble{
	
	int xPos;
	int yPos;
	int colour;
	int upRow;
	int rightCol;
	int leftCol;
	bool travelingRight;
	
} LaunchBubble;


////////////INITIALIZER FUNCTIONS////////////
void initialize_game_board();
void reset_launch_bubble();
void reset_visited();
void initialize_angle_array();
double get_angle();
/////////////////////////////////////////////

////////DRAWING AND ERASING FUNCTIONS////////
void draw_bubble(int xc, int yc, int r, int colour);
void draw_bubble_boundary(int xc, int yc, int x, int y, int colour);
void erase_bubble(int row, int col);
void erase_launch(int xc, int yc);
void check_pop(int row, int col);
/////////////////////////////////////////////

////////////UPDATE GAME FUNCTIONS////////////
void update_game_board(int row, int col);
void shift_game_board();
bool check_game_over();
void set_closest_row_and_col();
/////////////////////////////////////////////

///////////COUNT DOWN FUNCTIONS ////////////
bool count_down();
void display_hex(int i);
/////////////////////////////////////////////

///////////ARROW FUNCTIONS///////////////////
void initialize_arrow();
void draw_arrow();
void clear_arrow();
////////////////////////////////////////////

////////////USER INPUT FUNCTIONS//////////////
void user_input();
/////////////////////////////////////////////

///////////////HELPER FUNCTIONS//////////////
void draw_line(int x0, int y0, int x1, int y1, short int line_color);
void swap(int* number1, int* number2);
void wait_loop();
void plot_pixel(int x, int y, short int line_color);
void clear_screen();
/////////////////////////////////////////////


/////////////////GLOBAL VARIABLES/////////////////
Bubble game_board[12][16];
LaunchBubble launch;
bool visited[11][16];
int bubble_colour[3] = {0xC8A4FF, 0xF81F, 0xFF876D};
int entered_recursive;
bool keep_going = false;
int posx, posy;
int size = 15;
Pair angle_array[7];
int count = 0;
int lastCount;
bool toRight = true;
bool toLeft = false;
bool travelingRight[15];
//get the key 0 bit
int key0;

volatile int pixel_buffer_start;
volatile int pixel_buffer_back;
volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
volatile int* pixel_status_ptr = (int *)0xFF20302C;
volatile int *hex3_0_ptr = (int *)0xFF200020;
volatile int * key_ptr = (int *)0xFF200050;

//////////////////////////////////////////////////

int main(void)
{
	pixel_buffer_start = *pixel_ctrl_ptr;
	pixel_buffer_back = *pixel_ctrl_ptr;
		
	clear_screen();
	
	initialize_game_board();

	initialize_angle_array();
	
	bool game_over = false;
	int drawLaunchRow, drawLaunchCol;

	//counting down on HEX
	//bool dropDown = count_down();

	while(!game_over){

		//reset to wait for user input
		keep_going = false;
		bool bounce = false;

		reset_launch_bubble();
		
		reset_visited();
		
		// initialize_arrow();
		// draw_bubble(launch.xPos, launch.yPos, 10, launch.colour);

		// key0 = *(key_ptr) && 0x1;
		
		// while(key0 != 0){
		// 	key0 = *(key_ptr) && 0x1;
		// }
		
		// user input should produce a signal of whether to keep going or not
		user_input();

		posy = 230;
		posx = 169;
		
		
		while(keep_going){

			wait_loop();
			erase_launch(launch.xPos, launch.yPos);

			if(bounce){
				launch.yPos -= angle_array[count].row;
				launch.xPos += angle_array[count].col;
				launch.travelingRight = travelingRight[count];
				posy -= angle_array[count].row;
				posx += angle_array[count].col;
				//check if next position would go out of bound
				if( (posx - angle_array[count].col <= 0) || posx - angle_array[count].col >= 319 ){
					bounce = false;
				}

			}else{
				launch.yPos -= angle_array[count].row;
				launch.xPos -= angle_array[count].col;
				launch.travelingRight = travelingRight[count];
				posy -= angle_array[count].row;
				posx -= angle_array[count].col;
				//check if next position would go out of bound
				if( (posx - angle_array[count].col <= 0) || posx - angle_array[count].col >= 319 ){
					bounce = true;
				}
			}
			
			set_closest_row_and_col();
			
			if(game_board[launch.upRow][launch.rightCol].colour != 0x0
			&& launch.travelingRight){
				drawLaunchRow = launch.upRow + 1;
				drawLaunchCol = launch.rightCol - 1;
				keep_going = false;
			}
			else if(game_board[launch.upRow][launch.leftCol].colour != 0x0
			&& !launch.travelingRight){
				drawLaunchRow = launch.upRow + 1;
				drawLaunchCol = launch.leftCol + 1;
				keep_going = false;
			}

			draw_bubble(launch.xPos, launch.yPos, 10, launch.colour);	
			draw_arrow();
			
		}

		erase_launch(launch.xPos, launch.yPos);
		update_game_board(drawLaunchRow, drawLaunchCol);
		game_over = check_game_over();
		if(entered_recursive < 2)
			shift_game_board();
		
	}
	
	return 0;
}

/////////////////////////////////////INITIALIZER FUNCTIONS/////////////////////////////////////

void initialize_game_board(){
	
	int xCount = 0;
	int yCount = 0;
	
	for(int i = 0; i < 11; i++){
		for(int j = 0; j < 16; j++){
			
		
			game_board[i][j].xc = 20 * j + 9;
			game_board[i][j].yc = 10 + (20 * i);
			game_board[i][j].colour = bubble_colour[rand() % 3];
			game_board[i][j].radius = 10;
			game_board[i][j].visible = true;
			
			if(i == 0){
				game_board[i][j].up.row = -1;
				game_board[i][j].up.col = -1;
			}
			else{
				game_board[i][j].up.row = i - 1;
				game_board[i][j].up.col = j;
				
			}
				
			if(i == 10){
				game_board[i][j].down.row = -1;
				game_board[i][j].down.col = -1;
			}
			else{
				game_board[i][j].down.row = i + 1;
				game_board[i][j].down.col = j;
			}
			
			if(j == 0){
				game_board[i][j].left.row = -1;
				game_board[i][j].left.col = -1;
			}
			else{
				game_board[i][j].left.row = i;
				game_board[i][j].left.col = j -1;
			}
				
			if(j == 15){
				game_board[i][j].right.row = -1;
				game_board[i][j].right.col = -1;
			}
			else{
				game_board[i][j].right.row = i;
				game_board[i][j].right.col = j + 1;
			}
			
			if(i > 2){
				game_board[i][j].colour = 0x0;
				game_board[i][j].visible = false;
			}
			
			 draw_bubble(game_board[i][j].xc, game_board[i][j].yc, game_board[i][j].radius, game_board[i][j].colour);
			
			if(j == 15){
				xCount += 16;
				yCount += 1;
			}
		
		}
		
	}
	
	// initialize_arrow();
}

void reset_launch_bubble(){
	
	launch.xPos = 169;
	launch.yPos = 230;
	launch.colour = bubble_colour[rand() % 3];
	launch.upRow = 11;
	launch.rightCol = 8;
}

void reset_visited(){
	
	for(int i = 0; i < 11; i++)
		for(int j = 0; j < 16; j++)
			visited[i][j] = false;
		
	entered_recursive = 0;
}

void initialize_angle_array(){
	
	//the middle 
	angle_array[3].row = 5;
	angle_array[3].col = 0;
	//going straight

	// the right half ??
	angle_array[0].row = 5;
	angle_array[0].col = -4;

	angle_array[1].row = 5;
	angle_array[1].col = -2;

	angle_array[2].row = 5;
	angle_array[2].col = -1;

	angle_array[4].row = 5;
	angle_array[4].col = 1;

	angle_array[5].row = 5;
	angle_array[5].col = 2;

	angle_array[6].row = 5;
	angle_array[6].col = 4;

	
	for(int i = 0; i < 7; i++)
		if(angle_array[i].col < 3)
			travelingRight[i] = true;

}


/////////////////////////////////END OF INITIALIZER FUNCTIONS/////////////////////////////////


////////////////////////////////DRAW AND POP BUBBLE FUNCTIONS/////////////////////////////////

void draw_bubble(int xc, int yc, int r, int colour){
 
    int x = 0, y = r; 
    int d = 3 - 2 * r; 

    draw_bubble_boundary(xc, yc, x, y, colour); 
    while (y >= x) { 
        x++; 
		
        if (d > 0) { 
            y--;  
            d = d + 4 * (x - y) + 10; 
        } 
        else
            d = d + 4 * x + 6; 
        draw_bubble_boundary(xc, yc, x, y, colour);

    }
}
	
void draw_bubble_boundary(int xc, int yc, int x, int y, int colour){ 
	draw_line(xc - x, yc + y, xc + x, yc + y, colour);
	draw_line(xc - x, yc - y, xc + x, yc - y, colour);
	draw_line(xc - y, yc + x, xc + y, yc + x, colour);
	draw_line(xc - y, yc - x, xc + y, yc - x, colour);
}

void erase_bubble(int row, int col){
	game_board[row][col].visible = false;
	game_board[row][col].colour = 0x0;
	initialize_arrow();
	draw_bubble(game_board[row][col].xc, game_board[row][col].yc, game_board[row][col].radius, 0x0);
}

void erase_launch(int xc, int yc){
	initialize_arrow();
	draw_bubble(xc, yc, 10, 0x0);
}

void check_pop(int row, int col){
	
//	if(row < 0 || col < 0)
//		return;
	
	entered_recursive++;
	
	visited[row][col] = true;
	bool entered_at_least_once = false;
	
	if(game_board[row][col].colour == game_board[game_board[row][col].up.row][game_board[row][col].up.col].colour){
		entered_at_least_once = true;
		check_pop(game_board[row][col].up.row, game_board[row][col].up.col);
	}
	if(game_board[row][col].colour == game_board[game_board[row][col].right.row][game_board[row][col].right.col].colour){
		entered_at_least_once = true;
		check_pop(game_board[row][col].right.row, game_board[row][col].right.col);
	}
	if(game_board[row][col].colour == game_board[game_board[row][col].left.row][game_board[row][col].left.col].colour
	&& visited[game_board[row][col].left.row][game_board[row][col].left.col] == false){
		entered_at_least_once = true;
		check_pop(game_board[row][col].left.row, game_board[row][col].left.col);
	}

	if(entered_recursive > 2)	
		erase_bubble(row, col);
	
}

/////////////////////////////END OF DRAW AND POP BUBBLE FUNCTIONS//////////////////////////////



/////////////////////////////////UPDATE GAME BOARD FUNCTIONS//////////////////////////////////

void update_game_board(int row, int col){
	
	game_board[row][col].colour = launch.colour;
	game_board[row][col].visible = true;
	erase_launch(launch.xPos, launch.yPos);
	draw_bubble(game_board[row][col].xc, game_board[row][col].yc, 10, game_board[row][col].colour);
	check_pop(row, col);
}

void shift_game_board(){
	
	Bubble copy_of_game_board[12][16];
	
	for(int j = 0; j < 16; j++)
		copy_of_game_board[0][j].colour = bubble_colour[rand() % 3];
		
	for(int i = 1; i < 11; i++)
		for(int j = 0; j < 16; j++)
			copy_of_game_board[i][j].colour = game_board[i-1][j].colour;
			
	for(int i = 0; i < 11; i++)
		for(int j = 0; j < 16; j++)
			game_board[i][j].colour = copy_of_game_board[i][j].colour;
			
	//update_game_board();
	//clear_screen();
	
	for(int i = 0; i < 11; i++)
		for(int j = 0; j < 16; j++)
			draw_bubble(game_board[i][j].xc, game_board[i][j].yc, 10, game_board[i][j].colour);
	
	
}

bool check_game_over(){
	
	for(int j = 0; j < 16; j++)
		if(game_board[10][j].colour != 0x0)
			return true;
			
	return false;
}

void set_closest_row_and_col(){
	
	//setting row
	printf("launch y is %d\n", launch.yPos);
	
	//deal with row 0
	for(int i = 0; i < 11; i++){
		
		if(launch.yPos - 10 <= (i + 1) * 20 
		&& launch.yPos - 10 > i * 20) 
			launch.upRow = i;
	}
	printf("closest row is %d\n", launch.upRow);
	
	//setting cols
	printf("launch x is %d\n", launch.xPos);
	for(int j = 0; j < 16; j++){
		if(launch.xPos + 10 >= 300)
			launch.rightCol = 15;
		else if(launch.xPos + 10 >= j * 20
		&& launch.xPos + 10 < (j + 1) * 20)
			launch.rightCol = j;
		if(launch.xPos - 10 <= 20)
			launch.leftCol = 0;
		else if(launch.xPos - 10 > j * 20
		&& launch.xPos - 10 <= (j + 1) * 20)
			launch.leftCol = j;
	}
	printf("closest right col is %d\n", launch.rightCol);
	printf("closest left col is %d\n", launch.leftCol);
	
	
}

/////////////////////////////END OF UPDATE GAME BOARD FUNCTIONS//////////////////////////////


//////////////////////////////////////HELPER FUNCTIONS///////////////////////////////////////

void wait_loop(){
	*pixel_ctrl_ptr = 1;
	do{
	}while(*pixel_status_ptr & 0x01);
}

void swap(int* number1, int* number2){
  int temp = *number1;
  *number1 = *number2;
  *number2 = temp;
}

void draw_line(int x0, int y0, int x1, int y1, short int line_color){
	
	int x, y, deltax, deltay, error, y_step;
	bool isSteep = abs(y1 - y0) > abs(x1 - x0);
	
	if(isSteep){
		swap(&x0,&y0);
        swap(&x1,&y1);
	}
	
	if(x0 > x1){
		swap(&x0, &x1);
		swap(&y0, &y1);
	}
	
	deltax = x1 - x0;
	deltay = abs(y1 - y0);
	error = -(deltax / 2);
	y = y0;
	if(y0 < y1)
		y_step = 1;
	else
		y_step = -1;
	for(x = x0; x < x1; x++){
		if(isSteep)
			plot_pixel(y, x, line_color);			
		else
			plot_pixel(x, y, line_color);
		error += deltay;
		if(error >= 0){
			y += y_step;
			error -= deltax;
		}
	}
}

void plot_pixel(int x, int y, short int line_color){
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void clear_screen(){
	
	memset((short int*) pixel_buffer_start, 0, 245760 );
}

//////////////////////////////////END OF HELPER FUNCTIONS/////////////////////////////////

//////////////////////////////////COUNTER FUNCTION //////////////////////////////////////

bool count_down(){

	//counts down from 10
	int displayNum = 10;
	//approxumately 1 sec if we count down with this number
	int ticksPerSecond = 1000000;

	while(1){
		//exit once counted to 0
		if( displayNum < 0){
			return true;
		}else{
			display_hex(displayNum);
			//loop that makes sure the hex changes every second
			while(ticksPerSecond >= 0){
				ticksPerSecond--;
			}
			displayNum --;
			ticksPerSecond = 1000000;
		}
	}
	return false;
}

//display number on hex
void display_hex(int i){

	//seg 7 code from 0 to 9
	//one char is one byte
	char seg7[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67};

	if(i >= 10){
		*(hex3_0_ptr) = (seg7[i/10]<<8 | seg7[i%10]);
	}else{
		*hex3_0_ptr = seg7[i];
	}
}

/////////////////////////////////END OF COUNTER FUCNTION///////////////////////////////

////////////////////////////////ARROW FUNCTIONS////////////////////////////////////////

void initialize_arrow(){

	for(int y = 200; y < 240; y++){

		if(y >= 200 & y < 205){

			for(int x = 168; x < 169; x++){
				plot_pixel(x, y, 0xE7AE);
			}

		}else if( (y >= 205) && (y < 240)){

			for(int x = 167; x < 170; x++){
				plot_pixel(x, y, 0xE7AE);
			}
		}
	}
}

void draw_arrow(){

	//keep in mind that > 7 is left 
	if(count == 7) initialize_arrow();

	if(count == 6){
		draw_line( 135 , 199 , 167, 239, 0xE7AE);
		draw_line( 136 , 199 , 168, 239, 0xE7AE);
		draw_line( 137 , 199 , 169, 239, 0xE7AE);
	}

	if(count == 5){
		draw_line( 151 , 199 , 167, 239, 0xE7AE);
		draw_line( 152 , 199 , 168, 239, 0xE7AE);
		draw_line( 153 , 199 , 169, 239, 0xE7AE);
	}
	
	if(count == 4){
		draw_line( 159 , 199 , 167, 239, 0xE7AE);
		draw_line( 160 , 199 , 168, 239, 0xE7AE);
		draw_line( 161 , 199 , 169, 239, 0xE7AE);
	}	

	if(count == 2){
		draw_line( 175 , 199 , 167, 239, 0xE7AE);
		draw_line( 176 , 199 , 168, 239, 0xE7AE);
		draw_line( 177 , 199 , 169, 239, 0xE7AE);
	}

	if(count == 1){
		draw_line( 183 , 199 , 167, 239, 0xE7AE);
		draw_line( 184 , 199 , 168, 239, 0xE7AE);
		draw_line( 185 , 199 , 169, 239, 0xE7AE);
	}

	if(count == 0){
		draw_line( 199 , 199 , 167, 239, 0xE7AE);
		draw_line( 200 , 199 , 168, 239, 0xE7AE);
		draw_line( 201 , 199 , 169, 239, 0xE7AE);
	}

}

void clear_arrow(){
		//keep in mind that > 7 is left 
	if(lastCount == 7){
			for(int y = 200; y < 240; y++){

			if(y >= 200 & y < 205){

				for(int x = 168; x < 169; x++){
					plot_pixel(x, y, 0x0);
				}

			}else if( (y >= 205) && (y < 240)){

				for(int x = 167; x < 170; x++){
					plot_pixel(x, y, 0x0);
				}
			}
		}
	}

	if(lastCount == 6){
		draw_line( 135 , 199 , 167, 239, 0x0);
		draw_line( 136 , 199 , 168, 239, 0x0);
		draw_line( 137 , 199 , 169, 239, 0x0);
	}

	if(lastCount == 5){
		draw_line( 151 , 199 , 167, 239, 0x0);
		draw_line( 152 , 199 , 168, 239, 0x0);
		draw_line( 153 , 199 , 169, 239, 0x0);
	}
	
	if(lastCount == 4){
		draw_line( 159 , 199 , 167, 239, 0x0);
		draw_line( 160 , 199 , 168, 239, 0x0);
		draw_line( 161 , 199 , 169, 239, 0x0);
	}	

	if(lastCount == 2){
		draw_line( 175 , 199 , 167, 239, 0x0);
		draw_line( 176 , 199 , 168, 239, 0x0);
		draw_line( 177 , 199 , 169, 239, 0x0);
	}

	if(lastCount == 1){
		draw_line( 183 , 199 , 167, 239, 0x0);
		draw_line( 184 , 199 , 168, 239, 0x0);
		draw_line( 185 , 199 , 169, 239, 0x0);
	}

	if(lastCount == 0){
		draw_line( 199 , 199 , 167, 239, 0x0);
		draw_line( 200 , 199 , 168, 239, 0x0);
		draw_line( 201 , 199 , 169, 239, 0x0);
	}
}

////////////////////////////////END OF ARROW FUNCTIONS/////////////////////////////////

/////////////////////////////////USER INPUT FUNCTIONS/////////////////////////////////

void user_input(){

	int timer = 1000000/25;
	
	//go through the loop to make key is pressed
	key0 = *(key_ptr) && 0x1;

	do{
		
		if( toRight & key0 == 0 ){
			lastCount = count;
			count = count + 1;

			timer = 1000000/4;
			while(timer > 0){
				timer--;
			}
			//switch counting up or down
			if(count == size-1){
				toRight = false;
				toLeft = true;
			}

		}else if( toLeft & key0 == 0){
			//switch counting up or down
			lastCount = count;
			count = count - 1;

			timer = 1000000/4;
			while(timer > 0){
				timer--;
			}
			if(count == 0){
				toRight = true;
				toLeft = false;
			}
		}
		
		clear_arrow();
		wait_loop();
		draw_arrow();
		draw_bubble(launch.xPos, launch.yPos,10, launch.colour);
		
		keep_going = false;

		key0 = *(key_ptr) & 0x1;
	}while(key0 == 0);

	keep_going = true;
	clear_arrow();
	draw_arrow();
	draw_bubble(launch.xPos, launch.yPos,10, launch.colour);
	// draw_arrow();
	//go through the loop to make sure the key is unpressed
	// while(key0 == 1){
		
	// 	if(key0 == 0){
	// 		keep_going = true;
	// 	}

	// 	key0 = *(key_ptr) && 0x1;
	// 	//keep_going = false;
	// }

	
}

///////////////////////////////END OF USER INPUT FUNCTIONS////////////////////////////



