# pop3-email-networking
## Email service based on a simplified POP3 protocol is implemented. Both server side and client side applications are provided and multiple clients are supported. This project was done as part of CS224, Computer Networks, Spring 2019, IIT Bombay.

### Description :

The project was submitted in 4 *phases*. These are explained as follows:
1. Phase 1 - 1 client, basic connection establishment, hello and quit.
2. Phase 2 - 1 client, Lists the number of available messages for the given user.
3. Phase 3 - 1 client, Full file transfer.
4. Phase 4 - Multiple simultaneous clients, supporting all of the above.


### Conventions :
The server and the client(of Phase 4) require fairly self-explanatory command line arguments. Other phases' arguments are a subset of these.

The server needs to be provided the following:
1. Port number on which to start server
2. Password file : A password file which contains newline (`\n`) separated user-password entries. Each entry contains username followed by a space followed by password.
3. User-database : A folder containing different user folders (*mailboxes*). Each user folder must have the same name as the username and the files within each folder **must** be of the form `<positive-integer>.<three-letter-file-extension` (constraint imposed by the project. For example, `1.jpg`, `234.PDF`, etc.

The client needs to be provided the following:
1. Server IP address and port in the form : `<serverIPaddr:port>` . For eg, `10.0.0.1:8000`
2. Username
3. Password
4. List of messages to be retrieved. This must be a comma-separated list of integers. For example, `1,2,4`. It is assumed one already knows the list of available messages.
5. Path to a local folder to store the messages. **WARNING:** If this folder already exists, it will be forcefully removed and all its contents will be deleted.
6. Interval. : Duration in seconds to sleep between each message request.

### How to run :
~~~~
g++ --std=c++11 SimpleEmailServerPhase*.cpp -o server
g++ --std=c++11 SimpleEmailClientPhase*.cpp -o client
./server <args>
./client <args>
