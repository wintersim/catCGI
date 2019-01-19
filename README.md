# catCGI
[Caturbate's](http://caturbate.ga) CGI script.

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
