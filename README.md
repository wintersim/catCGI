# catCGI
[Catgasm's](http://catgasm.cc) CGI script.

## Installation
* Clone this repository
* Make sure you have all required libraries installed properly
* If needed, change the database and/or log path in ```database.h``` and ```logger.h```
* Compile it using ```cmake .``` followed by ```make```
* Copy the executable to your desired location
* e.g. ```/usr/lib/cgi-bin/catAjax```
* Configure your webserver to allow CGI scripts
* Example nginx config (place it somewhere in your 'server' block)
```
location /cgi-bin/ {
                gzip off;
                root /usr/lib;
                fastcgi_pass unix:/var/run/fcgiwrap.socket;
                include /etc/nginx/fastcgi_params;
                fastcgi_param SCRIPT_FILENAME $document_root$fastcgi_script_name;
        }
```
* Create your database with the ```createDb.sh``` script and copy it to the path you specified
* DONE

## Usage
```html
<img src="cgi-bin/catCgi?type=cat">
```


## Libraries used
* [sodium](https://libsodium.gitbook.io/doc/)
* [sqlite3](https://www.sqlite.org/index.html)
* [kcgi](https://kristaps.bsd.lv/kcgi/)
* [cURL](https://curl.haxx.se/libcurl/)


###### Notice
If someone actually has set this up, please let me know :) Right now this is more like an instruction for myself so I don't forget.
