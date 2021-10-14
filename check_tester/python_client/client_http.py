import http.client
import json

def client_get(ip, port, file_name):
    print ("----------------------------")
    print("\n\n")
    print(ip + ":" + str(port))
    print("\n");

    connect = http.client.HTTPConnection(ip, port)
    connect.request("GET", file_name)

    response = connect.getresponse()
    print (response.version, response.status, response.reason)

    header = response.getheaders()
    for i in header:
        print (i)

    print (response.read().decode())
    print ("----------------------------")
    connect.close()


def client_delete(ip, port, file_name):
    print ("----------------------------")
    print("\n\n")
    print(ip + ":" + str(port))
    print("\n");

    connect = http.client.HTTPConnection(ip, port)
    connect.request("DELETE", file_name)

    response = connect.getresponse()
    print (response.version, response.status, response.reason)

    header = response.getheaders()
    for i in header:
        print (i)

    print (response.read().decode())
    print ("----------------------------")
    connect.close()


def client_post(ip, port, file_name):
    print ("----------------------------")
    print("\n\n")
    print(ip + ":" + str(port))
    print("\n");

    connect = http.client.HTTPConnection(ip, port)

    headers = { 'Content-type': 'text/plain',
                'Content-Length': '5'
                }
    body = "filrl"

    connect.request('POST', file_name, body,  headers)


    response = connect.getresponse()
    print (response.version, response.status, response.reason)

    header = response.getheaders()
    for i in header:
        print (i)

    print (response.read().decode())
    print ("----------------------------")
    connect.close()


    '''
    headers = {'Content-type': 'application/json'}

    foo = {'text': 'Hello HTTP #1 **cool**, and #1!'}

    json_data = json.dumps(foo)

    connect.request('POST', '/post', json_data, headers)

    response = connect.getresponse()
    print(response.read().decode())

    connect.close()
    '''



#___main____________________________________________


my_serv = "127.0.0.1"
port_m = 9000

#client_post(my_serv, port_m, "/post/test.txt")
#client_get(my_serv, port_m, "/cgi-bin/hello.py/with/additional/path?and=a&query=string")
#client_delete(my_serv, port_m, "/delete/4")
#client_get(my_serv, port_m, "YoupiBanane")
client_get(my_serv, port_m, "/directory")
