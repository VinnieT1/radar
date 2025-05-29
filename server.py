from http.server import BaseHTTPRequestHandler, HTTPServer
import json

class SimplePostHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        content_length = int(self.headers.get('Content-Length', 0))
        body = self.rfile.read(content_length)

        try:
            data = json.loads(body)
            print("Parsed JSON:")
            print(json.dumps(data, indent=2))

            self.send_response(200)
            self.send_header('Content-Type', 'text/plain')
            self.end_headers()
            self.wfile.write(b'Received')
        except Exception as e:
            print("Body is not valid JSON or is empty.")
            print(f"Error: {e}")

            self.send_response(400)
            self.send_header('Content-Type', 'text/plain')
            self.end_headers()
            self.wfile.write(b'Invalid JSON')

def run(server_class=HTTPServer, handler_class=SimplePostHandler, port=8080):
    server_address = ('', port)
    httpd = server_class(server_address, handler_class)
    print(f"Starting HTTP server on port {port}...")
    httpd.serve_forever()

if __name__ == "__main__":
    run()
