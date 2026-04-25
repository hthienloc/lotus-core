import http.server
import json
import os

PORT = 8080
INBOX_FILE = os.path.join(os.path.dirname(__file__), 'bridge_inbox.json')

class BridgeHandler(http.server.BaseHTTPRequestHandler):
    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()

    def do_POST(self):
        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length)
        data = json.loads(post_data.decode('utf-8'))
        
        # Save to inbox
        with open(INBOX_FILE, 'w', encoding='utf-8') as f:
            json.dump(data, f, ensure_ascii=False, indent=2)
        
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()
        self.wfile.write(b'{"status": "ok"}')
        print(f"[*] Received message from Jules session: {data.get('url')}")

if __name__ == '__main__':
    print(f"🚀 Gemini Bridge Relay starting on http://localhost:{PORT}")
    http.server.HTTPServer(('localhost', PORT), BridgeHandler).serve_forever()
