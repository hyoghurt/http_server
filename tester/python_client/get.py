import  http.client
import  sys

if len(sys.argv) != 3:
    print ("Error: get.py www.example.com /")
    sys.exit(1)


conn = http.client.HTTPSConnection(sys.argv[1])
conn.request("GET", sys.argv[2])
r2 = conn.getresponse()

print (r2.version, r2.status, r2.reason)

header = r2.getheaders()
for i in header:
    print (i)

print ()
print(r2.read().decode())
conn.close()
