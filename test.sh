#curl -I -X GET http://127.0.0.1:9000/
#curl -I -X POST http://127.0.0.1:9000/
#curl -I -X HEAD http://127.0.0.1:9000/
#curl -I -X GET http://127.0.0.1:9000/directory
#curl -I -X GET http://127.0.0.1:9000/directory/youpi.bla
#curl -I -X GET http://127.0.0.1:9000/directory/Yeah
#curl -I -X GET http://127.0.0.1:9000/directory/Yeah/
#curl -i -X POST -d "lol kek" http://127.0.0.1:9000/post_body/testic
#curl -i -X PUT -d @text.txt  http://127.0.0.1:9000/put_test/file_should_exist_after
#curl -i -X POST -d @text.txt http://127.0.0.1:9000/directory/youpi.bla
curl -i -X POST -d lolkek http://127.0.0.1:9000/directory/youpi.bla
