#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define LAUNCH_ROW 11
#define LAUNCH_COL 12

////////////INITIALIZER FUNCTIONS////////////
void initialize_game_board();
void reset_launch_bubble();
void reset_visited();
/////////////////////////////////////////////

////////DRAWING AND ERASING FUNCTIONS////////
void draw_bubble(int xc, int yc, int r, int colour, bool pop);
void draw_bubble_boundary(int xc, int yc, int x, int y, int colour);
void erase_bubble(int row, int col);
void pop_bubble(int row, int col);
void check_pop(int row, int col);
/////////////////////////////////////////////

////////////UPDATE GAME FUNCTIONS////////////
void update_game_board();
void shift_game_board();
bool check_game_over();
/////////////////////////////////////////////

///////////////HELPER FUNCTIONS//////////////
void draw_line(int x0, int y0, int x1, int y1, short int line_color);
void swap(int* number1, int* number2);
void wait_loop();
void plot_pixel(int x, int y, short int line_color);
void clear_screen();
/////////////////////////////////////////////


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

/////////////////GLOBAL VARIABLES/////////////////
Bubble game_board[12][16];
bool visited[11][16];
int bubble_colour[3] = {0xC8A4FF, 0xF81F, 0xFF876D};
int entered_recursive;

volatile int pixel_buffer_start;
volatile int pixel_buffer_back;
volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
volatile int* pixel_status_ptr = (int *)0xFF20302C;
volatile int *hex3_0_ptr = (int *)0xFF200020;
//////////////////////////////////////////////////

int main(void)
{
	pixel_buffer_start = *pixel_ctrl_ptr;
	pixel_buffer_back = *pixel_ctrl_ptr;
		
	clear_screen();
	
	initialize_game_board();
	
	bool game_over = false;
	
	while(!game_over){
		
		reset_launch_bubble();
		
		reset_visited();
		
		draw_bubble(game_board[LAUNCH_ROW][LAUNCH_COL].xc, game_board[LAUNCH_ROW][LAUNCH_COL].yc, game_board[LAUNCH_ROW][LAUNCH_COL].radius, game_board[LAUNCH_ROW][LAUNCH_COL].colour, false);
		
		int y = 230;
		bool keep_going = true;
		
		while(keep_going){
			for(int j = 0; j < 16; j++){
				for(int i = 0; i < 11; i++){
					if(game_board[LAUNCH_ROW][LAUNCH_COL].xc == game_board[i][j].xc 
					&& game_board[i][j].yc >= (game_board[LAUNCH_ROW][LAUNCH_COL].yc - 25)
					&& game_board[i][j].yc <= (game_board[LAUNCH_ROW][LAUNCH_COL].yc + 25)
					&& game_board[i][j].colour != 0x0)
						keep_going = false;
				}
			}

			wait_loop();
			erase_bubble(LAUNCH_ROW, LAUNCH_COL);
			game_board[LAUNCH_ROW][LAUNCH_COL].yc -= 3;
			y -= 3;
			draw_bubble(169, y, 10, game_board[LAUNCH_ROW][LAUNCH_COL].colour, false);

		
		}
		update_game_board();
		game_over = check_game_over();
		if(entered_recursive < 2)
			shift_game_board();
		//reset_launch_bubble();
		
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
			
			draw_bubble(game_board[i][j].xc, game_board[i][j].yc, game_board[i][j].radius, game_board[i][j].colour, false);
			
			if(j == 15){
				xCount += 16;
				yCount += 1;
			}
		
		}
		
	}
	
}


void reset_launch_bubble(){
	
	game_board[LAUNCH_ROW][LAUNCH_COL].xc = 169;
	game_board[LAUNCH_ROW][LAUNCH_COL].yc = 230;
	game_board[LAUNCH_ROW][LAUNCH_COL].radius = 10;
	game_board[LAUNCH_ROW][LAUNCH_COL].colour = bubble_colour[rand() % 3];
	game_board[LAUNCH_ROW][LAUNCH_COL].visible = true;
}

void reset_visited(){
	
	for(int i = 0; i < 11; i++)
		for(int j = 0; j < 16; j++)
			visited[i][j] = false;
		
	entered_recursive = 0;
}

/////////////////////////////////END OF INITIALIZER FUNCTIONS/////////////////////////////////


////////////////////////////////DRAW AND POP BUBBLE FUNCTIONS/////////////////////////////////

void draw_bubble(int xc, int yc, int r, int colour, bool pop){
 
    int x = 0, y = r; 
    int d = 3 - 2 * r; 
	if(pop)
		colour = 0x0;
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
		
		if(pop)
			wait_loop();

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
	if(row != LAUNCH_ROW && col != LAUNCH_COL)
		game_board[row][col].colour = 0x0;
	draw_bubble(game_board[row][col].xc, game_board[row][col].yc, game_board[row][col].radius, 0x0, false);
}

void pop_bubble(int row, int col){
	game_board[row][col].visible = false;
	game_board[row][col].colour = 0x0;
	draw_bubble(game_board[row][col].xc, game_board[row][col].yc, game_board[row][col].radius, 0x0, true);
}

void check_pop(int row, int col){
	
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

void update_game_board(){
	for(int i = 0; i < 11; i++){
		for(int j = 0; j < 16; j++){	
			if(game_board[LAUNCH_ROW][LAUNCH_COL].xc == game_board[i][j].xc 
				&& game_board[LAUNCH_ROW][LAUNCH_COL].yc >= game_board[i][j].yc - 2
				&& game_board[LAUNCH_ROW][LAUNCH_COL].yc <= game_board[i][j].yc + 2){
					game_board[i][j].colour = game_board[LAUNCH_ROW][LAUNCH_COL].colour;
					erase_bubble(LAUNCH_ROW, LAUNCH_COL);
					draw_bubble(game_board[i][j].xc, game_board[i][j].yc, 10, game_board[i][j].colour, false);
					check_pop(i, j);
			}
		}
	}
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
			draw_bubble(game_board[i][j].xc, game_board[i][j].yc, 10, game_board[i][j].colour, false);
	
	
}

bool check_game_over(){
	
	for(int j = 0; j < 16; j++)
		if(game_board[10][j].colour != 0x0)
			return true;
			
	return false;
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
	
	for(int i = 0; i < 320; i++)
		for(int j = 0; j < 240; j++ )
			plot_pixel(i, j, 0x0);
}

//////////////////////////////////END OF HELPER FUNCTIONS/////////////////////////////////

void display_hex(){
	//char zero = 0b00111111, one = 0b00000110, two = 0b01011011, three = 0b01001111, four = 0b01100110;
	//char five = 0b01101101, six = 0b01111101, seven = 0b00000111, eight = 0b01111111, nine = 0b01100111;
}
