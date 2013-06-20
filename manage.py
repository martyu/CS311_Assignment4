import SocketServer

class TCPHandler(SocketServer.StreamRequestHandler):

    def handle(self):
        while(1):
            self.data = self.rfile.readline()
            print "%s wrote:" % self.client_address[0]
            print self.data
            self.request.send('you sent' + self.data.upper())

if __name__ == "__main__":
    HOST, PORT = "", 50001

    # Create the server, binding to localhost on port 50001
    server = SocketServer.TCPServer((HOST, PORT), TCPHandler)

    # Activate the server; this will keep running until you
    # interrupt the program with Ctrl-C
    server.serve_forever()
