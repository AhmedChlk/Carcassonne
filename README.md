13/06/2025

TO REMAKE

    * Make sure that the first tile in the board is  {p, r, c, r, r}
        * In *initialize_board()* we can place the first tile in board[height // 2][width // 2] then we shuffle the pack and we remove the tile from it (71 instead of 72) 
        * We can initilize the game with a specific function *init_game()* where we can do what it's mentionned below.

    * Verify that we can place a tile when we have shield and city
        * A shield can match with city. Same, same but different in the points we gain when we close a struct

TO DO NEXT

    * Make a function to calculate the score once we have a closed struct 
        * Determine the closed struct
            * CITY
            * ROAD
            * ABBEY
                * It is sufficient to check that the adjacent positions aren't empty to say that we have a complete abbey

        NOTE : 
            * Verify only the tiles which have a meeple 
            * Define a struct storing tiles positions and meeple position  