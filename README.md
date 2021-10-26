# http_server
Compile: `make`    
Run: `./webserv [arg]`  

Program have a config file in argument or use a default path.  
Server listen on multiple ports.  
Methods: GET, POST, and DELETE.  

### setup config file:
```
server:  
host: 127.0.0.1  
port: 9000  
server_name: example.ru  
error_page: 400 400.html  

location /  
root: www  
index: index.html  
autoindex: on  
access_method: GET POST  
return: 301 [URL]  
cgi_pass: python3  
client_max_body_size: 100
```
![terminal](https://github.com/hyoghurt/http_server/raw/master/terminal.png)
