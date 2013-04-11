The TCP File Transporting Program
=================================

Directory Description
---------------------

- `recv_files` The default receive directory.
- `server_db` The database of server which contains all files in server.
- `source` The source code directory

Usage
-----

1. Enter into `source` directory and `make`.

2. For Server:

    Put the test files into `server_db` and run `./file_server`.

3. For Client:

    > `./trans_client` address:port request_file [recv_directory]
