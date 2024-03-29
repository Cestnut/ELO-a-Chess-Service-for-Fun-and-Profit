#include "chess.h"
#include "common.h"
#include "common_conf.h"
#define DEBUG FALSE

int parse(char *charset, int charset_len, char letter);
int parse_column(char letter);
int parse_row(char letter);

board_struct *init_board(){

    char clean_board[BOARD_SIZE][BOARD_SIZE][3] = { 
    { "wR", "wP", "..", "..", "..", "..", "bP", "bR" },
    { "wN", "wP", "..", "..","..", "..", "bP", "bN" },
    { "wB", "wP", "..", "..","..", "..", "bP", "bB" },
    { "wQ", "wP", "..", "..","..", "..", "bP", "bQ" },
    { "wK", "wP", "..", "..","..", "..", "bP", "bK" },
    { "wB", "wP", "..", "..","..", "..", "bP", "bB" },
    { "wN", "wP", "..", "..","..", "..", "bP", "bN" },
    { "wR", "wP", "..", "..", "..", "..", "bP", "bR" }
    };

    board_struct *board = malloc(sizeof(board_struct));
    for(int col=0; col<BOARD_SIZE; col++){
        for(int row=0; row<BOARD_SIZE; row++){
            piece_struct *piece = (piece_struct*)malloc(sizeof(piece_struct));

            char color_char, type_char;
            color_char = clean_board[col][row][0];
            type_char = clean_board[col][row][1];

            switch(color_char){
                case 'w':
                    piece->color = WHITE;
                    break;
                case 'b':
                    piece->color = BLACK;
                    break;
                case '.':
                    piece->color = NO_COLOR;
            }
            switch(type_char){
                case '.':
                    piece->type = NO_TYPE;
                    break;
                case 'P':
                    piece->type = PAWN;
                    break;
                case 'R':
                    piece->type = ROOK;
                    break;
                case 'B':
                    piece->type = BISHOP;
                    break;
                case 'N':
                    piece->type = KNIGHT;
                    break;
                case 'K':
                    piece->type = KING;
                    break;
                case 'Q':
                    piece->type = QUEEN;
                    break;
            }
            piece->moved_flag = FALSE;
            board->board[col][row] = piece;

        }
    }
    return board;
}

void render_board(board_struct *board, piece_color player_color){
    piece_struct *piece;
    if(player_color == BLACK){
        for(int row=0; row<BOARD_SIZE; row++){
            printf("%d ", row+1);
            for(int col=BOARD_SIZE-1; col>=0; col--){
                piece = board->board[col][row];
                char symbol;
                if(piece->color == NO_COLOR) symbol = '.';
                else if(piece->color == WHITE) symbol = WHITE_CHARSET[piece->type];
                else if(piece->color == BLACK) symbol = BLACK_CHARSET[piece->type];

                printf("%3c ", symbol);
            }
            printf("\n");
        }
    }
    else{
        for(int row=BOARD_SIZE-1; row>=0; row--){
            printf("%d ", row+1);
            for(int col=0; col<BOARD_SIZE; col++){
                piece = board->board[col][row];
                char symbol;
                if(piece->color == NO_COLOR) symbol = '.';
                else if(piece->color == WHITE) symbol = WHITE_CHARSET[piece->type];
                else if(piece->color == BLACK) symbol = BLACK_CHARSET[piece->type];

                printf("%3c ", symbol);
            }
            printf("\n");
        }
    }

    char columns[BOARD_SIZE];
    if(player_color == BLACK) memcpy(columns, "HGFEDCBA", BOARD_SIZE);
    else if(player_color == WHITE) memcpy(columns, "ABCDEFGH", BOARD_SIZE);
    printf("  ");
    for(int i=0; i<sizeof(columns); i++){
        printf("%3c ", columns[i]);
    }
    printf("\n\n");
}

Position get_king_position(board_struct *board, piece_color player_color){
    piece_struct *piece;
    Position point;
    int found=FALSE;
    int row, col;
    for(row=0; row<BOARD_SIZE; row++){
        for(col=0; col<BOARD_SIZE; col++){
            piece = board->board[col][row];
            if(piece->color == player_color && piece->type == KING){
                point.col = col;
                point.row = row;
                found = TRUE;
                break;
            }
        }
        if(found) break;
    }
    if (DEBUG) printf("found:%d, row:%d, col:%d", found, row, col);

    return point;
}

//If the move would lead to being in check
int is_move_safe(board_struct *board, piece_color player_color, Position src_position, Position dst_position){
    if (DEBUG) printf("Check if move is safe\n");
    int result;
    piece_struct tmp_piece, *moving_piece;
    //Make the move but save the piece in destination, since we may have to rollback
    tmp_piece = *board->board[dst_position.col][dst_position.row];
    moving_piece = board->board[src_position.col][src_position.row];
    *board->board[dst_position.col][dst_position.row] = *moving_piece;
    moving_piece->color = NO_COLOR;
    moving_piece->type = NO_TYPE;
    if(is_in_check(board, player_color)){
        result = FALSE;
    }
    else{
        result = TRUE;
    }
    
    //Rollback the movement
    *moving_piece = *board->board[dst_position.col][dst_position.row];
    *board->board[dst_position.col][dst_position.row] = tmp_piece;
    if (DEBUG) printf("Rolled back movement\n");

    return result;
}

void move_piece(board_struct *board, Position src_position, Position dst_position){
    //Set moved flag to true
    piece_struct *src_piece, *dst_piece;
    src_piece = board->board[src_position.col][src_position.row];
    dst_piece = board->board[dst_position.col][dst_position.row];
    
    //Copy source piece to dst piece, and set moved flag
    *dst_piece = *src_piece;
    dst_piece->moved_flag = TRUE;

    //Make src piece empty
    src_piece->color=NO_COLOR;
    src_piece->type=NO_TYPE;


    //If moved piece was pawn, promote to queen in case it reached the end of the board
    if(dst_piece->type == PAWN){
        int last_row;
        if(dst_piece->color == WHITE) last_row = 7;
        else if(dst_piece->color == BLACK) last_row = 0;

        if(dst_position.row == last_row) dst_piece->type = QUEEN;
    }
    else if(dst_piece->type == KING){
        int col_variation = dst_position.col - src_position.col;
        Position rook_src, rook_dst;
        if(col_variation == +2){
            rook_src.col = 7;
            rook_src.row = src_position.row;

            rook_dst.col = dst_position.col - 1;
            rook_dst.row = src_position.row;
            move_piece(board, rook_src, rook_dst);
        }
        else if(col_variation == -2){
            rook_src.col = 0;
            rook_src.row = src_position.row;
            
            rook_dst.col = dst_position.col + 1;
            rook_dst.row = src_position.row;
            move_piece(board, rook_src, rook_dst);
        }
    }
}

int is_move_valid(board_struct *board, piece_color player_color, Position src_position, Position dst_position){
    if (DEBUG) printf("Trying move %d%d-%d%d\n", src_position.col, src_position.row, dst_position.col, dst_position.row);
    if(is_pattern_valid(board, player_color, src_position, dst_position) && is_move_safe(board, player_color, src_position, dst_position)){
            return TRUE;
    }
    else{
        return FALSE;
    }
}

int is_in_check(board_struct *board, piece_color player_color){
    if (DEBUG) printf("Is in check\n");
    piece_color opponent_color;
    piece_struct *piece;
    Position king_position = get_king_position(board, player_color), src_position;
    if(player_color == WHITE) opponent_color=BLACK;
    else if(player_color == BLACK) opponent_color = WHITE;
    if (DEBUG) printf("Player color: %d\nOpponent color: %d\n", player_color, opponent_color);
    for(int row=0; row<BOARD_SIZE; row++){
        for(int col=0; col<BOARD_SIZE; col++){
            piece = board->board[col][row];
            if(piece->color == opponent_color){
                src_position.col = col;
                src_position.row = row;
                if (DEBUG) printf("Seeing if player %d can do move %d%d-%d%d", opponent_color, col,row, king_position.col, king_position.row);
                if(is_pattern_valid(board, opponent_color, src_position, king_position)){
                    if(DEBUG) printf("This would put the king in check from %d%d that could go to %d%d\n", col, row, king_position.col, king_position.row);
                    return TRUE;
                }
            }
        }
    }
    return FALSE;

}

int has_valid_moves(board_struct *board, piece_color player_color){
    Position src_position, dst_position;
    for(int src_col=0; src_col < BOARD_SIZE; src_col++){
        src_position.col = src_col;
        for(int src_row=0; src_row < BOARD_SIZE; src_row++){
            src_position.row = src_row;
            for(int dst_col=0; dst_col < BOARD_SIZE; dst_col++){
                dst_position.col = dst_col;
                for(int dst_row=0; dst_row < BOARD_SIZE; dst_row++){
                    dst_position.row = dst_row;
                    if(is_move_valid(board, player_color, src_position, dst_position)){
                        if (DEBUG) printf("%d%d-%d%d is an example of valid move\n", src_col, src_row, dst_col, dst_row);
                        return TRUE;
                    }
                }
            }
        }
    }
    return FALSE;
}


int is_pattern_valid(board_struct *board, piece_color player_color, Position src_position, Position dst_position){
    piece_struct *src_piece = board->board[src_position.col][src_position.row];
    piece_struct *dst_piece = board->board[dst_position.col][dst_position.row];
    if (DEBUG) printf("Validating pattern\n");
    //if destination is outside board
    if(dst_position.col >= BOARD_SIZE || dst_position.row >= BOARD_SIZE || dst_position.col < 0 || dst_position.row < 0) return FALSE;

    //if piece is not being moved
    if(src_position.col==dst_position.col && src_position.row == dst_position.row) return FALSE;

    //if moved piece is not owned by player
    if(src_piece->color != player_color) return FALSE;

    //if destination piece is owned by player
    if(dst_piece->color == player_color) return FALSE;

    switch(src_piece->type){
        case PAWN:
            return is_pattern_valid_pawn(board, player_color, src_position, dst_position);
            break;
        case ROOK:
            return is_pattern_valid_rook(board, player_color, src_position, dst_position);            
            break;
        case KNIGHT:
            return is_pattern_valid_knight(board, player_color, src_position, dst_position);
            break;
        case BISHOP:
            return is_pattern_valid_bishop(board, player_color, src_position, dst_position);
            break;
        case KING:
            return is_pattern_valid_king(board, player_color, src_position, dst_position);
            break;
        case QUEEN:
            return is_pattern_valid_queen(board, player_color, src_position, dst_position);
            break;
        default:
            return FALSE;
            break;
    }
}


int is_pattern_valid_pawn(board_struct *board, piece_color player_color, Position src_position, Position dst_position){
    piece_struct *src_piece = board->board[src_position.col][src_position.row];
    piece_struct *dst_piece = board->board[dst_position.col][dst_position.row];    
    piece_color opponent_color;
    if(player_color == WHITE) opponent_color=BLACK;
    else if(player_color == BLACK) opponent_color=WHITE;
    
    int step; //Used because white and black pawns go forward in opposing directions
    if(player_color == WHITE) step=1;
    else step = -1;

    //Checks if destination row is 1 row space forward and square is empty
    //else if destination row is 2 spaces forward and piece hasn't moved yet and square is empty
    if(dst_position.row == src_position.row+step){
        //If destination column is the same and there are no pieces there 
        //else if destination column is shifted of one and there is an opponent's piece there
        if(src_position.col == dst_position.col && dst_piece->type==NO_TYPE){
            return TRUE;
        }
        else if((src_position.col == dst_position.col + 1 || src_position.col == dst_position.col - 1) && dst_piece->color==opponent_color){
            return TRUE;
        }
        else return FALSE;
    }
    else if(src_position.col == dst_position.col && dst_position.row == src_position.row+(2*step) && src_piece->moved_flag == 0 && board->board[dst_position.col][src_position.row+step]->type==NO_TYPE && dst_piece->type==NO_TYPE){
        return TRUE;
    }
    else{
        return FALSE;
    } 
}
int is_pattern_valid_rook(board_struct *board, piece_color piece_color, Position src_position, Position dst_position){
    int step; //Represents the direction of movement (-1 or +1)
    //If movement is straight (only one coordinate is changing)
    if(src_position.row==dst_position.row){
        //Here we control if the path to destination is clear
        if(src_position.col > dst_position.col) step = -1;
        else step = +1;

        //current position to analyze, since movement happens along column
        src_position.col += step;
        while(src_position.col != dst_position.col){
            //If in the current cell there is already a piece, return false
            if(board->board[src_position.col][src_position.row]->type != NO_TYPE) return FALSE;
            src_position.col += step;
        }
        return TRUE;
    }
    else if(src_position.col==dst_position.col){
        if(src_position.row > dst_position.row) step = -1;
        else step = +1;

        src_position.row += step;
        while(src_position.row != dst_position.row){
            if(board->board[src_position.col][src_position.row]->type != NO_TYPE) return FALSE;
            src_position.row += step;
        }
        return TRUE;

    }
    else{
        return FALSE;
    }    
}

int is_pattern_valid_knight(board_struct *board, piece_color player_color, Position src_position, Position dst_position){
    //Movement is valid if one of the two coordinates changes by 1 and the other by 2.
    //Knight is the only piece that doesn't care if the path is obstructed
    if(dst_position.col == src_position.col + 2 || dst_position.col == src_position.col - 2){
        if(dst_position.row == src_position.row +1 || dst_position.row == src_position.row -1) return TRUE;
        else return FALSE;
    }
    else if(dst_position.col == src_position.col +1 || dst_position.col == src_position.col -1){
        if(dst_position.row == src_position.row +2 || dst_position.row == src_position.row -2) return TRUE;
        else return FALSE;
    }
    else{
        return FALSE;
    }
}

int is_pattern_valid_bishop(board_struct *board, piece_color player_color, Position src_position, Position dst_position){
    //Movement is valid if change of row and column are of the same magnitude (absolute value of variation)
    int row_variation, col_variation, row_step, column_step;
    row_variation = dst_position.row - src_position.row;
    col_variation = dst_position.col - src_position.col;
    if(row_variation * row_variation == col_variation * col_variation){
        //Direction of movement on row
        if(dst_position.row > src_position.row) row_step = +1;
        else row_step = -1;
        //Direction of movement on column
        if(dst_position.col > src_position.col) column_step = +1;
        else column_step = -1;

        src_position.row += row_step;
        src_position.col += column_step;
        //We check step by step if the path is occupied
        while(src_position.row != dst_position.row){
            if(board->board[src_position.col][src_position.row]->type != NO_TYPE) return FALSE;
            src_position.row += row_step;
            src_position.col += column_step;
        }
        return TRUE;
    }
    else return FALSE;

}


int is_pattern_valid_queen(board_struct *board, piece_color player_color, Position src_position, Position dst_position){
    return is_pattern_valid_rook(board, player_color, src_position, dst_position) || is_pattern_valid_bishop(board, player_color, src_position, dst_position);
}

int is_pattern_valid_king(board_struct *board, piece_color player_color, Position src_position, Position dst_position){
    //Movement is valid only if row or column change by one
    int row_variation, col_variation;
    row_variation = dst_position.row - src_position.row;
    col_variation = dst_position.col - src_position.col;

    //If movement is in adjacent cells
    //else if movement is horizontal by 2, check castling conditions
    if(row_variation>=-1 && row_variation <=1 && col_variation>= -1 && col_variation <= 1) return TRUE;
    else if(row_variation==0 && (col_variation == 2 || col_variation == -2)){
        piece_struct *rook, *king;
        Position tmp_position = src_position;
        king = board->board[src_position.col][src_position.row];
        
        //King can't castle if its under check
        if(is_in_check(board, player_color)) return FALSE;
        else if(col_variation == 2){
            rook = board->board[7][src_position.row];
            tmp_position.col += 1; //Column of the square the king passes through 

            //King and rook should never have been moved
            if(rook->moved_flag || king->moved_flag) return FALSE;
            //Path has to be clear
            else if(board->board[tmp_position.col][tmp_position.row]->type != NO_TYPE) return FALSE;
            //The square the king passes through has to be safe
            else if(!is_move_safe(board, player_color, src_position,tmp_position)) return FALSE;
            else return TRUE;

        }
        else if(col_variation == -2){
            rook = board->board[0][src_position.row];
            tmp_position.col -= 1; 
            if(rook->moved_flag || king->moved_flag) return FALSE;
            else if(board->board[tmp_position.col][tmp_position.row]->type != NO_TYPE) return FALSE;
            else if(!is_move_safe(board, player_color, src_position,tmp_position)) return FALSE;
            //For movement to the left an additional square has to be empty since the path is longer
            else if(board->board[tmp_position.col - 2][tmp_position.row]->type != NO_TYPE) return FALSE;
            else return TRUE;
        }
    }
    else return FALSE;

    return FALSE;
}

int parse(char *charset, int charset_len, char letter){
    for(int i=0; i<charset_len; i++){
        if(toupper(charset[i]) == toupper(letter)){
            return i;
        }
    }
    return -1;
}

int parse_column(char letter){
    return parse("ABCDEFGH", 8, letter);
}

int parse_row(char letter){
    return parse("12345678", 8, letter);
}

Position *parse_move(char *move_string){
    Position *positions = malloc(sizeof(Position)*2);
    int parsed_letter;
    //This is done to avoid that the original string gets modified
    char buffer_copy[BUFFER_LEN];
    strncpy(buffer_copy, move_string, sizeof(buffer_copy));
    char *token;

    //PARSE FIRST POINT
    token = strtok(buffer_copy, "-");
    if(token==NULL){
        free(positions);
        return NULL;
    } 

    parsed_letter = parse_column(token[0]);
    if(parsed_letter == -1){
        free(positions);
        return NULL;
    }
    positions[0].col = parsed_letter;
    
    parsed_letter = parse_row(token[1]);
    if(parsed_letter == -1){
        free(positions);
        return NULL;
    }
    positions[0].row = parsed_letter;
    
    //PARSE SECOND POINT
    token = strtok(NULL, "");
    if(token==NULL){
        free(positions);
        return NULL;
    }

    parsed_letter = parse_column(token[0]);
    if(parsed_letter == -1){
        free(positions);
        return NULL;
    }
    positions[1].col = parsed_letter;
    
    parsed_letter = parse_row(token[1]);
    if(parsed_letter == -1){
        free(positions);
        return NULL;
    }
    positions[1].row = parsed_letter;


    return positions;
}

void clean_board(board_struct *board){
    for(int col=0; col<BOARD_SIZE; col++){
        for(int row=0; row<BOARD_SIZE; row++){
            free(board->board[col][row]);
        }
    }
    free(board);
}