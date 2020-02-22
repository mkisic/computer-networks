[Link to university class page](https://web.math.pmf.unizg.hr/nastava/mreze/)

##Pacman

My goal was to create server and client network applications which will be able to play multiplayer Pacman game.
Rules of the game were following:
....1. Server will read from standard input number of players (up to 10) and the map.
....2. Server is waiting players to connect
....3. First player who connects with server is Pacman and his mark is "@". Other players are marked from "1" to "9".
....4. When all players are connected, game starts. Server sends the map to players.
....5. Players are making moves by inputing UDLR to standard input.
....6. When some player makes a move, server will update the map and send it to all players.
....7. Game stops when some player and Pacman are on same field. At the end, server will send notification about the end of game and the number of steps which Pacman made it.
