
g++ -c *.cpp -lraylib -lm -D_REETRANT 
g++ -o fluid *.o -lraylib -lm -D_REETRANT 
rm *.o
./fluid
