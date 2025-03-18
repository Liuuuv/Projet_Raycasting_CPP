# projet
 
// v1 //
// int wallHeight = (int)(HEIGHT / (distance + 0.0001));
// int drawStart = (HEIGHT - wallHeight) / 2 + player.pitch*0;
// int drawEnd = drawStart + wallHeight;

// v2 //
// float wallHeight =  1.0f;    // Hauteur du mur dans le jeu
// float wallProportionScreen = wallHeight/ (distance * tanf(player.verticalFOV/2.0f));

// int wallHeightScreen = (int)floor(wallProportionScreen * HEIGHT);
// int drawStart = HEIGHT/2 - (int)floor(wallProportionScreen * HEIGHT) + player.pitch*100;
// int drawEnd = drawStart + wallHeightScreen;

EXECUTER 3d.cpp suffit.
