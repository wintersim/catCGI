# catCGI
[Catgasm's](http://catgasm.cc) CGI script.

## Usage
```javascript
var xhttp = new XMLHttpRequest();
xhttp.open("POST", "/cgi-bin/catCgi", true);
xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
        document.getElementById("your-container").innerHTML = this.responseText;
    }
};
xhttp.send('cat');
```

## Libraries used
* [sodium](https://libsodium.gitbook.io/doc/)
* [sqlite3](https://www.sqlite.org/index.html)
* [kcgi](https://kristaps.bsd.lv/kcgi/)
