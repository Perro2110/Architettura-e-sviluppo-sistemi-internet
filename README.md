# Socket Programming in C e Java

## Architettura Client-Server
```
      SERVER                          CLIENT
    +---------+                    +---------+
    |         |       TCP/IP       |         |
    | process |<==================>| process |
    |         |                    |         |
    +---------+                    +---------+
    bind()+listen()                connect()
```

## Socket in C
Le socket in C rappresentano il modo tradizionale di programmare la comunicazione di rete.

### Flusso TCP
```
    SERVER                      CLIENT
    socket()                    socket()
       |                          |
    bind()                        |
       |                          |
    listen()                      |
       |                          |
    accept() <----------------- connect()
       |                          |
    read() <------------------ write()
       |                          |
    write() -----------------> read()
       |                          |
    close() <----------------> close()
```

### Creazione Socket in C
```c
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
```
- `AF_INET`: famiglia di protocolli IPv4
- `SOCK_STREAM`: per TCP
- `SOCK_DGRAM`: per UDP

### Server TCP in C
1. **Creazione socket**
2. **Bind**: associa la socket a un indirizzo
3. **Listen**: mette la socket in ascolto
4. **Accept**: accetta connessioni in arrivo

```c
struct sockaddr_in server_addr;
bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
listen(sockfd, 5);  // 5 è la dimensione della coda
new_socket = accept(sockfd, (struct sockaddr*)&client_addr, &addr_len);
```

### Client TCP in C
1. **Creazione socket**
2. **Connect**: connessione al server

```c
connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
```

## Socket in Java

### Modello TCP vs UDP
```
    TCP                             UDP
+------------+                 +------------+
|  Reliable  |                 | Unreliable |
| Ordered    |                 | Unordered  |
| Stream     |                 | Datagram   |
| Heavy      |                 | Light      |
+------------+                 +------------+
      |                             |
   [Socket]                  [DatagramSocket]
```

### TCP Socket in Java
Java fornisce due classi principali per TCP:
- `ServerSocket`: per il lato server
- `Socket`: per il lato client

#### Server TCP Java
```java
ServerSocket serverSocket = new ServerSocket(port);
Socket clientSocket = serverSocket.accept();
PrintWriter out = new PrintWriter(clientSocket.getOutputStream(), true);
BufferedReader in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
```

#### Client TCP Java
```java
Socket socket = new Socket("localhost", port);
PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
```

### Flusso UDP
```
    SERVER UDP                 CLIENT UDP
       |                          |
    [bind]                     [send]
       |                          |
    [receive] <----------------- /
       |                        /
    [process]                  /
       |                      /
    [send] ----------------->/
```

### DatagramSocket (UDP) in Java
Per la comunicazione UDP, Java fornisce:
- `DatagramSocket`: per inviare e ricevere pacchetti
- `DatagramPacket`: rappresenta il pacchetto da inviare/ricevere

#### Server UDP Java
```java
DatagramSocket socket = new DatagramSocket(port);
byte[] buffer = new byte[1024];
DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
socket.receive(packet);  // blocca finché non riceve dati
```

#### Client UDP Java
```java
DatagramSocket socket = new DatagramSocket();
InetAddress address = InetAddress.getByName("localhost");
byte[] buffer = "Hello".getBytes();
DatagramPacket packet = new DatagramPacket(buffer, buffer.length, address, port);
socket.send(packet);
```

## Confronto TCP vs UDP
```
+------------------+        +------------------+
|       TCP        |        |       UDP        |
+------------------+        +------------------+
|   Connection     |        |   Connectionless |
|   Oriented      <+        |   Oriented       |
|   [=========]    |        |   [...]          |
+------------------+        +------------------+
|   Reliable       |        |   Best Effort    |
|   Ordered        |        |   Unordered      |
|   Flow Control   |        |   No Flow Ctrl   |
+------------------+        +------------------+
|   Slower         |        |   Faster         |
|   Heavy          |        |   Light          |
+------------------+        +------------------+
```

## Differenze Principali

### TCP vs UDP
- **TCP**: 
  - Connessione orientata
  - Garantisce la consegna dei dati
  - Mantiene l'ordine dei pacchetti
  - Più lento ma affidabile

- **UDP**:
  - Senza connessione
  - Non garantisce la consegna
  - Non mantiene l'ordine
  - Più veloce ma meno affidabile

### C vs Java
- **C**: 
  - Più basso livello
  - Maggior controllo
  - Richiede gestione manuale della memoria
  - API più complessa

- **Java**:
  - Più alto livello
  - Gestione automatica della memoria
  - API più semplice e object-oriented
  - Cross-platform automatico
