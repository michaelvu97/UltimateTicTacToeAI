#include <iostream>
#include <string>
#include <sstream>

using namespace std;

/*
What makes a certain move strong?
1 we will win the game with that move [highest] [done]
2 we can win the sub in one move [done]
3 if it forces a move in which: [don't make right now]
  a) we win the whole game
  b) we win the box
  c) we get to move anywhere
4 we don't direct the opponent to a box they can win, unless (3). [done, kinda]
5 we block a three in a row [done]
6 we set up for a three in a row [done]
7 center [done]
8 corners [done]
9 edges [done]
*/

/*
 * GLOBAL VARIABLES
 */

// Whether we're X or O
//int GLOBAL_ourPlayerNumber;

/*
 * END GLOBAL VARIABLES
 */

/*
 * Function Declarations
 */
int getMoveScore(int board[9][9], int boxNumber, int cellNumber, int player);
int checkBoxStatus(int subBox[9]);
int checkImmediateWin(int subBox[9], int turn, int myTeam);
int checkOpponentMove(int box[9][9],int toCheck,int myTeam);
bool doesMoveLetOpponentTakeBox (int subBox[9], int currentTeam);
bool winGame(int superBox[9], int subBox[9], int turn, int myTeam, int boxNum);
bool checkSetForThree(int subBlock[9], int toTest, int currentPlayer);
bool cellUseless(int block[9], int position, int player);

/*
 * End funcion declarations.
 */

int getMoveScore(int board[9][9], int boxNumber, int cellNumber, int player) {
    /*
     * TODO
     * Called on a box to determine its value for the player.
     * Higher score = better.
     */

    // Arbitrary score weights
    const int SCORE_WINNING_MOVE    = 80085; // (. Y .)
    const int SCORE_WIN_SUB         = 15;
    const int SCORE_OPPO_WIN_SUB    = -15;
    const int SCORE_BLOCK_THREE     = 3;
    const int SCORE_SET_UP_3        = 2;
    const int SCORE_CENTER          = 1;
    const int SCORE_CORNER          = 2;
    const int SCORE_EDGE            = 3;

    int score = 0;

    /*
     * Check winning move
     */
    int boardStatus[9];
    for (int i = 0; i < 9; i++) {
        boardStatus[i] = checkBoxStatus(board[i]);
    }
    if (winGame(boardStatus, board[boxNumber], cellNumber, player, boxNumber))
        return SCORE_WINNING_MOVE;

    /*
     * Check if the move will win the sub.
     */
    if ( player == checkImmediateWin(board[boxNumber], cellNumber, player))
        score += SCORE_WIN_SUB;

    /*
     * Check if other player can win a sub.
     */
    if (doesMoveLetOpponentTakeBox (board[cellNumber], player))
        score += SCORE_OPPO_WIN_SUB;

    /*
     * Check if it will block the opponent from 3 in a row.
     */
    if (checkSetForThree(board[boxNumber], cellNumber, player))
        score += SCORE_BLOCK_THREE;

    /*
     * Check if it sets up 3
     */
    if (checkSetForThree(board[boxNumber], cellNumber, player))
        score += SCORE_SET_UP_3;


    /*
     * Is the move a center, corner, or edge?
     */
    if (cellNumber == 4)
        score += SCORE_CENTER;
    else if (cellNumber % 2 != 0)
        score += SCORE_EDGE;
    else
        score += SCORE_CORNER;

    return score;
}

int checkBoxStatus(int subBox[9]){
    /*
     * Returns which player has won the box, 3 means tie, 0 means none.
     */
    //x horizontal check
    for(int i=0;i<3;i++){
        if(subBox[i*3]==1&&subBox[i*3]==subBox[i*3+1]&&subBox[i*3]==subBox[i*3+2])
            return(1);
    }

    //x vertical check
    for(int i=0;i<3;i++){
        if(subBox[i]==1&&subBox[i]==subBox[i+3]&&subBox[i]==subBox[i+6])
            return(1);
    }

    //diagonal checks for x
    if(subBox[0]==1&&subBox[0]==subBox[4]&&subBox[0]==subBox[8]){
        return(1);
    }
    if(subBox[2]==1&&subBox[2]==subBox[4]&&subBox[2]==subBox[6]){
        return(1);
    }

    //o horizontal check
    for(int i=0;i<3;i++){
        if(subBox[i*3]==2&&subBox[i*3]==subBox[i*3+1]&&subBox[i*3]==subBox[i*3+2])
            return(2);
    }
    //o vertical check
    for(int i=0;i<3;i++){
        if(subBox[i]==2&&subBox[i]==subBox[i+3]&&subBox[i]==subBox[i+6])
            return(2);
    }

    //diagonal checks for x
    if(subBox[0]==2&&subBox[0]==subBox[4]&&subBox[0]==subBox[8]){
        return(2);
    }
    if(subBox[2]==2&&subBox[2]==subBox[4]&&subBox[2]==subBox[6]){
        return(2);
    }

    //tie checker
    for(int i=0;i<9;i++){
        if(subBox[i]==0)
            return(0);
    }

    // tie.
    return 3;
}

bool cellUseless(int block[9], int position, int player) {
    /*
     * Given a 3x3 block, is the cell at position useless for player?
     * Also: assume that the block can be played in (ie status = 0).
     */

    // Opponent id.
    int oppo = player == 1 ? 2 : 1;

    int col = position % 3;
    int row = position / 3;

    // Find if there exists any possibly win-states involving the cell.
    bool hasUse = false;

    // Search col
    if (!(block[col] == oppo || block[col + 3] == oppo || block[col + 6] == oppo))
        return false;
        
    // Search row
    if (!(block[3*row] == oppo || block[3*row + 1] == oppo || block[3*row + 2] == oppo))
        return false;

    // Search diagonal
    if (position % 2 == 0) {

        // is it the middle?
        if (position == 4)

            // check both diagonals, return true if both have at least one oppo.
            return (block[0] == oppo || block[8] == oppo) && 
                   (block[2] == oppo || block[6] == oppo);

        else if (position == 0 || position == 8)
            return (block[0] == oppo || block[4] == oppo || block[8] == oppo);
        else
            return (block[2] == oppo || block[4] == oppo || block[6] == oppo);
    }

    // There were no obstructions.
    return true;
}

int checkImmediateWin(int subBox[9], int turn, int myTeam){
    /*
     * Takes a selected box, a move, and current team.
     * Returns the box state if that move is made.
     */
    int status = 0;
    //make a copy of subBox and insert turn
    int copyBox[9];
    for(int i = 0; i < 9; i++){
        copyBox[i] = subBox[i];
    }
    copyBox[turn] = myTeam;
    //Check for matches
    status = checkBoxStatus(copyBox);
    return(status); //returns the status of the box if the move is made
}

bool doesMoveLetOpponentTakeBox (int subBox[9], int currentTeam) {
    /*
       * Checks if the current move will let the opponent take the following box.
       * Parameters: <subBox>,<the box that will be targeted>,<the current user's team.
       */
    
    int otherTeam = currentTeam == 1 ? 2 : 1;
    
    /*
     * Check if the cell has already been won or tied.
     * 
     * Basically, if the move sends the opponent to a free choice box, we don't want to
     * accidentally count it twice, since that will be checked elsewhere.
     */
    if (checkBoxStatus(subBox))
        return false;

    for (int i = 0 ; i < 9; i++) {
        if (subBox[i] == 0 && checkImmediateWin(subBox, i, otherTeam) == otherTeam)
            return true;
      }

    return false;

}

bool winGame(int superBox[9], int subBox[9], int turn, int myTeam, int boxNum){
    /*
     * Checks if a move will win the whole game for the team.
     * Checks if the move will win a subBox, if subBox is won,check if game is won
     * superBox is the status of each big box.
     */
    int status = checkImmediateWin(subBox, turn, myTeam);

    if(myTeam == status){
        int superStatus = checkImmediateWin(superBox, boxNum, myTeam);
        if(superStatus == myTeam)
            return true;
    }
    return false;
}
bool blockOpponentWin(int subBox[9], int turn, int myTeam){
    /*
       * Check if the possible move will prevent the opponent from winning a row
     */
    int opponent;
      bool blockWin = false;
      if(myTeam == 1)
        opponent = 2;
      else
        opponent = 1;
    //Replace move with opponents colour and see if the opponent wins the box
    int status = checkImmediateWin(subBox, turn, opponent);
      if (status == opponent)
            blockWin = true;
    return blockWin;
}

bool checkSetForThree(int subBox[9], int toTest, int currentPlayer){
    /* 
     * Check if adding a piece will result in one of the following patterns in any direction:
       * NN0, N0N, 0NN
     */
    // add the piece to the subbox

    int copyBox[9];
    for (int i = 0 ; i < 9; i++) {
        copyBox[i] = subBox[i];
    }

    // False if the spot is already occupied.
    if (subBox[toTest] != 0)
        return false;

    copyBox[toTest] = currentPlayer;
    
    int patternA = currentPlayer * 100 + currentPlayer;
    int patternB = currentPlayer * 10 + currentPlayer;
    int patternC = currentPlayer * 100 + currentPlayer * 10;
    
    int currentLine = 0;
    /*
       * Horizontals.
       */
    for (int i = 0; i < 3; i++) {
        currentLine = copyBox[3*i]*100 + copyBox[3*i + 1] * 10 + copyBox[3*i + 2];
        if (currentLine == 110 || currentLine == 101 || currentLine == 011)
            return true;
    }
    /*
     * Verticals.
     */
    for (int i = 0; i < 3; i++) {
        currentLine = copyBox[i]*100 + copyBox[i+3] * 10 + copyBox[i+6];
        if (currentLine == 110 || currentLine == 101 || currentLine == 011)
            return true;
    }
    
    /*
     * Diagonals
     */
    currentLine = copyBox[0]*100 + copyBox[4]*10 + copyBox[8];
    if (currentLine == 110 || currentLine == 101 || currentLine == 011)
        return true;
    currentLine = copyBox[2]*100 + copyBox[4]*10 + copyBox[6];
    if (currentLine == 110 || currentLine == 101 || currentLine == 011)
        return true;

    // Pattern not found, not a set up.
    return false;
}

int biggerBoardScoring(int status[9],int index, int team){
    double score =1.0; // base multiplier

    int copyArray[9];
    for(int i = 0; i<9; i++){
        copyArray[i]=status[i];
    }

    int opTeam = team == 1 ? 2 : 1;
    
    if (opTeam == checkImmediateWin(copyArray, index, opTeam))
        score = 0.25;
    if (opTeam == checkSetForThree(copyArray, index, opTeam))
        score = 0.66;
    if (cellUseless(copyArray,index,team))
        score = 2;
    return(score);
}

int main (int n, char **args) {
    
    int ourPlayerNumber =  args[2][0] - '0'; // Our player number
    int boxToPlay          =  args[2][1]; // Which box we must play in.
    int boxStatuses[9];
    int boxScore[9];


    // Populate board
    int board[9][9];

    bool boardIsEmpty = true; // keep track of if this is the first move.

    for (int i = 0 ; i < 81; i++) {
        board[i/9][i%9] = args[2][2 + i] - '0';
        if (boardIsEmpty && args[2][2+i] != '0')
            boardIsEmpty = false;
    }

    for(int i=0; i<9; i++){
        boxStatuses[i]=checkBoxStatus(board[i]);
    }
    for(int i=0; i<9; i++){
        if(boxStatuses[i]==0){
            boxScore[i]=biggerBoardScoring(boxStatuses,i,ourPlayerNumber);
        }
        else{
            boxScore[i]=1;
        }
    }

    if (boardIsEmpty) {
        // We are on the first move, play middle middle.
        cout << (4 * 9) + 4;
        return 0;
    }

    if (boxToPlay == '9') {

        // We can play anywhere.

        int maxScore = -1000;
        int maxCell  = 0;
        int maxBox   = 0;

        for(int i=0;i<9;i++){
            if (checkBoxStatus(board[i]) == 0) {
                for (int j = 0; j < 9; j++) {
                    if (board[i][j] == 0) {
                        int tempScore = getMoveScore(board, i, j, 
                                                    ourPlayerNumber);
                        if (maxScore < tempScore) {

                            maxScore = tempScore;
                            maxBox = i;
                            maxCell = j;
                        }
                    }
                }
            }
        }

        // We now have the max score at box,cell.
        cout << (9 * maxBox) + maxCell;
        return 0;

    }

    // At this point, we are confined to one box.
    
    int currentBoxForMove = args[2][1] - '0';

    int maxScore = -1000;
    int maxCell  = 0;

    for (int i = 0 ; i < 9; i++) {
        // Make sure cell isn't occupied.
        if (board[currentBoxForMove][i] == 0) {
            int tempScore = getMoveScore(board, currentBoxForMove, i, 
                                                ourPlayerNumber);
            tempScore=tempScore*boxScore[i];
            if(boxStatuses[i]!=0){
                tempScore=tempScore-15;
            }
            if (maxScore < tempScore) {
                
                // Replace the max.
                maxCell = i;
                maxScore = tempScore;

            }

        }

    }
    
    // Output our final decision.
    cout << (currentBoxForMove * 9) + maxCell;  
    
    return 0; 
}
