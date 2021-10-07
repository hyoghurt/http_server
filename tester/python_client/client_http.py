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



def client_post(ip, port):
    print ("\n\npost\n\n")
    connect = http.client.HTTPConnection(ip, port)

    headers = {'Content-type': 'text/plain'}

    foo = "loli_pop";

    connect.request('POST', '/post', foo, headers)

    response = connect.getresponse()
    print (response.version, response.status, response.reason)

    header = response.getheaders()
    for i in header:
        print (i)

    print(response.read().decode())

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

#client_post(my_serv, port_m)
#client_get(my_serv, port_m, "/cgi-bin/hello.php/with/additional/path?and=a&query=string")
#client_get(my_serv, port_m, "/cgi-bin/hello.php/with/additional/path?and=a&query=string")
client_delete(my_serv, port_m, "/delete/del.h")
