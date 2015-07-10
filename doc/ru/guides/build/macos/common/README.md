new page
=============================================


cd LinuxDrone/tools  
git clone git://git.libwebsockets.org/libwebsockets  
cd libwebsockets  
mkdir build  
cd build  
cmake ..  
make  
sudo make install  

{@img gem_error.png}


How to fix error and get back to https with ssl:

The reason is old rubygems. So we need to remove ssl source to be able to update gem --system which includes rubygems and so on. after this we can feel free to get back to ssl source.

gem sources -r https://rubygems.org/ - to temporarily remove secure connection

gem sources -a http://rubygems.org/ - add insecure connection

sudo gem update --system - now we're able to update rubygems without SSL

after updating rubygems do vice versa

gem sources -r http://rubygems.org/ - to remove insecure connection

gem sources -a https://rubygems.org/ - add secure connection

Now you're able to update gems using secure connection.

sudo gem update



sudo gem install jsduck
