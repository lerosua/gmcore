g++ excgloox.cc -lgloox -lpthread -fpic -shared -g -o libexcgloox.so 
sudo cp libexcgloox.so /usr/lib/
gcc main.c -g -lexcgloox  -o demo
