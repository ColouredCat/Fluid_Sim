
cd ~/emsdk
source ./emsdk_env.sh
cd ~/Documents/code/Fluid_Sim
em++ *.cpp -c -Os -Wall ./web/libraylib.a -I. -I/usr/include -I/usr/include/x86_64-linux-gnu -DPLATFORM_WEB
em++ -o sim.html *.o -Os -Wall ./web/libraylib.a -I. -L. -I/usr/include -I/usr/include/x86_64-linux-gnu -s USE_GLFW=3 --shell-file ./web/minshell.html --preload-file resorces  -s MIN_WEBGL_VERSION=2 -DPLATFORM_WEB
mv sim.* web
rm *.o
