#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

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
/////////////////////////////////////////////

////////DRAWING AND ERASING FUNCTIONS////////
void draw_bubble(int xc, int yc, int r, int colour);
void draw_bubble_boundary(int xc, int yc, int x, int y, int colour);
void erase_bubble(int row, int col);
void erase_launch(int xc, int yc);
void check_pop(int row, int col);
/////////////////////////////////////////////

////////////UPDATE GAME FUNCTIONS////////////
void update_game_board(int row, int col, int x, int y);
void shift_game_board();
void launch_bubble();
bool check_keep_going();
bool check_win();
bool check_game_over();
void check_move_up();
void fall();
/////////////////////////////////////////////

///////////COUNT DOWN FUNCTIONS ////////////
bool count_down();
void display_hex(int i);
/////////////////////////////////////////////

////////////USER INPUT FUNCTIONS//////////////
void move_launch();
/////////////////////////////////////////////

///////////////HELPER FUNCTIONS//////////////
void draw_line(int x0, int y0, int x1, int y1, short int line_color);
void swap(int* number1, int* number2);
void wait_loop();
void plot_pixel(int x, int y, short int line_color);
int closest_x();
int closest_y();
int row_from_pos(int yPos);
int col_from_pos(int xPos);
void clear_screen();
/////////////////////////////////////////////


/////////////////GLOBAL VARIABLES/////////////////
Bubble game_board[12][16];
LaunchBubble launch;
bool visited[11][16];
int bubble_colour[6] = {0x7C1F, 0xFA1F, 0x6EB9, 0xFFEF};
int entered_recursive;
bool hit_top;
bool keep_going;
int current_level;
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
	
	current_level = 1;
	
	initialize_game_board();
	
	bool game_over = false;
	bool winAll = false;
	bool winOne = false;

	//counting down on HEX
	//bool dropDown = count_down();

	while(!game_over || !winAll){

		//reset to wait for user input
		keep_going = false;

		reset_launch_bubble();
		reset_visited();
		
		draw_bubble(launch.xPos, launch.yPos, 10, launch.colour);

		key0 = *(key_ptr) && 0x1;
		
		while(key0 != 0){
			key0 = *(key_ptr) && 0x1;
		}
		
		// user input should produce a signal of whether to keep going or not
		//user_input();
		move_launch();
		
		erase_launch(launch.xPos, launch.yPos);
		launch.xPos = closest_x();
		
		draw_bubble(launch.xPos, launch.yPos, 10, launch.colour);

		
		
		while(keep_going){

			wait_loop();
			erase_launch(launch.xPos, launch.yPos);
			launch.yPos = launch.yPos - 3;
			draw_bubble(launch.xPos, launch.yPos, 10, launch.colour);
			
			keep_going = check_keep_going();
		}
		

		erase_launch(launch.xPos, launch.yPos);
		launch.yPos = closest_y();
		int row = row_from_pos(launch.yPos);
		int col = col_from_pos(launch.xPos);
		//check_move_up();
		update_game_board(row, col, launch.xPos - 1, launch.yPos);
		

		game_over = check_game_over();
		
		if(entered_recursive < 2 && !hit_top)
			shift_game_board();
			
		winOne = check_win();
		if(winOne){
			if(current_level == 3)
				winAll = true;
			else{
				current_level++;
				clear_screen();
				initialize_game_board();
			}
		}
		
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
			game_board[i][j].colour = bubble_colour[rand() % (current_level + 1)];
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
			
			if(i > current_level){
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
	
}

void reset_launch_bubble(){
	
	launch.xPos = 169;
	launch.yPos = 230;
	launch.colour = bubble_colour[rand() % (current_level + 1)];
	launch.upRow = 11;
	launch.rightCol = 8;
}

void reset_visited(){
	
	for(int i = 0; i < 11; i++)
		for(int j = 0; j < 16; j++)
			visited[i][j] = false;
		
	entered_recursive = 0;
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
	draw_bubble(game_board[row][col].xc, game_board[row][col].yc, game_board[row][col].radius, 0x0);
}

void erase_launch(int xc, int yc){
	draw_bubble(xc, yc, 10, 0x0);
}

void check_pop(int row, int col){
	
	entered_recursive++;
	
	visited[row][col] = true;
	
	if(game_board[row][col].colour != 0x0){
		if(game_board[row][col].up.row != -1 && game_board[row][col].up.col != -1)
			if(game_board[row][col].colour == game_board[game_board[row][col].up.row][game_board[row][col].up.col].colour){
				check_pop(game_board[row][col].up.row, game_board[row][col].up.col);
			}
			
		if(game_board[row][col].right.row != -1 && game_board[row][col].right.col != -1)
			if(game_board[row][col].colour == game_board[game_board[row][col].right.row][game_board[row][col].right.col].colour){
				check_pop(game_board[row][col].right.row, game_board[row][col].right.col);
			}
		if(game_board[row][col].left.row != -1 && game_board[row][col].left.col != -1)
			if(game_board[row][col].colour == game_board[game_board[row][col].left.row][game_board[row][col].left.col].colour
			&& visited[game_board[row][col].left.row][game_board[row][col].left.col] == false){
				check_pop(game_board[row][col].left.row, game_board[row][col].left.col);
			}
		if(game_board[row][col].down.row != -1 && game_board[row][col].down.col != -1)
			if(game_board[row][col].colour == game_board[game_board[row][col].down.row][game_board[row][col].down.col].colour
			&& visited[game_board[row][col].down.row][game_board[row][col].down.col] == false){
				check_pop(game_board[row][col].down.row, game_board[row][col].down.col);
			}
	}

	if(entered_recursive > 2)	
		erase_bubble(row, col);
	
}

/////////////////////////////END OF DRAW AND POP BUBBLE FUNCTIONS//////////////////////////////



/////////////////////////////////UPDATE GAME BOARD FUNCTIONS//////////////////////////////////

void update_game_board(int row, int col, int x, int y){
	game_board[row][col].colour = launch.colour;
	game_board[row][col].visible = true;
	draw_bubble(x, y, 10, game_board[row][col].colour);
	check_pop(row, col);
	//check_move_up();
}



void shift_game_board(){
	
	Bubble copy_of_game_board[12][16];
	
	for(int j = 0; j < 16; j++)
		copy_of_game_board[0][j].colour = bubble_colour[rand() % (current_level + 1)];
		
	for(int i = 1; i < 11; i++)
		for(int j = 0; j < 16; j++)
			copy_of_game_board[i][j].colour = game_board[i-1][j].colour;
			
	for(int i = 0; i < 11; i++)
		for(int j = 0; j < 16; j++)
			game_board[i][j].colour = copy_of_game_board[i][j].colour;
			
	
	for(int i = 0; i < 11; i++)
		for(int j = 0; j < 16; j++)
			draw_bubble(game_board[i][j].xc, game_board[i][j].yc, 10, game_board[i][j].colour);
	
	
}

void check_move_up(){
	
for(int i = 0; i < 11; i ++)
	for(int j = 0; j < 16; j++)
		while(game_board[game_board[i][j].up.row][game_board[i][j].up.col].colour == 0x0
		&& game_board[game_board[i][j].right.row][game_board[i][j].right.col].colour == 0x0
		&& game_board[game_board[i][j].left.row][game_board[i][j].left.col].colour == 0x0
		&& game_board[i][j].colour != 0x0)
			erase_bubble(i, j);
}

bool check_keep_going(){
	
	int j = col_from_pos(launch.xPos);
	
	
	if(launch.yPos - 12 <= 0){
		hit_top = true;
		return false;
	}

	for(int i = 0; i < 11; i++){
		if(game_board[i][j].yc > launch.yPos - 23
		&& game_board[i][j].yc < launch.yPos + 23
		&& game_board[i][j].colour != 0x0){
			hit_top = false;
			return false;
		}
	}

			
	return true;
}


bool check_win(){
	for(int i = 0; i < 11; i++)
		for(int j = 0; j < 16; j++)
			if(game_board[i][j].colour != 0x0)
				return false;
	return true;
}

bool check_game_over(){
	
	for(int j = 0; j < 16; j++)
		if(game_board[10][j].colour != 0x0)
			return true;
			
	return false;
}

//void fall(int row, int, col int colour){
//	
//	
//	
//}

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

int closest_x(){
	int x = 0;
	
	for(int j = 0; j < 16; j++)
		if(launch.xPos >= j * 20 && launch.xPos <= (j + 1) * 20)
			x = j * 20 + 10;
			
	return x;
	
}

int closest_y(){
	int y = 0;
	
	for(int j = 0; j < 11; j++)
		if(launch.yPos >= j * 20 && launch.yPos <= (j + 1) * 20)
			y = j * 20 + 10;
			
	return y;
	
}

int row_from_pos(int y){
	for(int i = 0; i < 11; i ++)
		if(y >= i*20 && y < (i+1)*20)
			return i;
	return 0;
	
}
int col_from_pos(int x){
	for(int i = 0; i < 16; i ++)
		if(x >= i*20 && x < (i+1)*20)
			return i;
	return 0;
	
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


////////////////////////////////END OF ARROW FUNCTIONS/////////////////////////////////

/////////////////////////////////USER INPUT FUNCTIONS/////////////////////////////////
	
void move_launch(){
	int direction = 3;
	while(key0 == 0){
		wait_loop();
		erase_launch(launch.xPos, launch.yPos);
		if(launch.xPos <= 0 || launch.xPos >= 318)
			direction = -1 * direction;
		launch.xPos = launch.xPos + direction;
		draw_bubble(launch.xPos, launch.yPos, 10, launch.colour);
		keep_going = true;
		key0 = *(key_ptr) && 0x1;
	}
	
}

///////////////////////////////END OF USER INPUT FUNCTIONS////////////////////////////



