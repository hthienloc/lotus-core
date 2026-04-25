import http.server
import json
import os

PORT = 8080
INBOX_FILE = os.path.join(os.path.dirname(__file__), 'bridge_inbox.json')
OUTBOX_FILE = os.path.join(os.path.dirname(__file__), 'bridge_outbox.json')

class BridgeHandler(http.server.BaseHTTPRequestHandler):
    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()

    def do_POST(self):
        """ Handles INBOX (Browser -> Gemini) """
        content_length = int(self.headers['Content-Length'])
        post_data = self.rfile.read(content_length)
        data = json.loads(post_data.decode('utf-8'))
        
        # Extract Session ID from URL
        url = data.get('url', '')
        session_id = url.split('/')[-1]
        data['session_id'] = session_id
        
        with open(INBOX_FILE, 'w', encoding='utf-8') as f:
            json.dump(data, f, ensure_ascii=False, indent=2)
        
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()
        self.wfile.write(b'{"status": "received"}')
        print(f"[*] Gemini received message from Session: {session_id}")

    def do_GET(self):
        """ Handles OUTBOX (Gemini -> Browser) """
        # Expecting path: /get_reply?session_id=123
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Content-Type', 'application/json')
        self.end_headers()

        if os.path.exists(OUTBOX_FILE):
            try:
                with open(OUTBOX_FILE, 'r', encoding='utf-8') as f:
                    outbox = json.load(f)
                
                # Check if this reply is meant for the requesting session
                # In real usage, we would parse query params, but here we just return the whole outbox
                # and let the Userscript decide.
                self.wfile.write(json.dumps(outbox).encode('utf-8'))
                return
            except:
                pass
        
        self.wfile.write(b'{}')

if __name__ == '__main__':
    # Ensure files exist
    if not os.path.exists(OUTBOX_FILE):
        with open(OUTBOX_FILE, 'w') as f: json.dump({}, f)
        
    print(f"🚀 Gemini Bridge Relay V2.0 starting on http://localhost:{PORT}")
    http.server.HTTPServer(('localhost', PORT), BridgeHandler).serve_forever()
