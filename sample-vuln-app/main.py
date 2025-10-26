from flask import Flask, request, render_template_string
import sqlite3
import os
import subprocess

app = Flask(__name__)

# Vulnerable HTML template
HTML_TEMPLATE = '''
<!DOCTYPE html>
<html>
<head>
    <title>Vulnerable Test App</title>
    
</head>
<body>
    <h1>Vulnerable Test Application</h1>
    
    <!-- SQL Injection Test -->
    <form method="POST" action="/search">
        <input type="text" name="query" placeholder="Search users...">
        <input type="submit" value="Search">
    </form>
    
    <!-- Command Injection Test -->
    <form method="POST" action="/ping">
        <input type="text" name="host" placeholder="Host to ping">
        <input type="submit" value="Ping">
    </form>
    
    <!-- XSS Test -->
    <form method="POST" action="/comment">
        <input type="text" name="comment" placeholder="Leave a comment">
        <input type="submit" value="Submit">
    </form>
    
    <!-- Path Traversal Test -->
    <form method="POST" action="/readfile">
        <input type="text" name="filename" placeholder="Filename to read">
        <input type="submit" value="Read File">
    </form>
    
    {% if results %}
    <h3>Results:</h3>
    <pre>{{ results }}</pre>
    {% endif %}
</body>
</html>
'''

def init_db():
    """Initialize a simple SQLite database"""
    conn = sqlite3.connect('test.db')
    cursor = conn.cursor()
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY,
            username TEXT,
            password TEXT
        )
    ''')
    # Add some test data
    cursor.execute("INSERT OR IGNORE INTO users (username, password) VALUES ('admin', 'password123')")
    cursor.execute("INSERT OR IGNORE INTO users (username, password) VALUES ('user1', 'pass456')")
    conn.commit()
    conn.close()

@app.route('/')
def index():
    return render_template_string(HTML_TEMPLATE)

# SQL Injection Vulnerability
@app.route('/search', methods=['POST'])
def search():
    query = request.form['query']
    conn = sqlite3.connect('test.db')
    cursor = conn.cursor()
    
    # VULNERABLE: Direct string concatenation
    cursor.execute(f"SELECT username FROM users WHERE username = '{query}'")
    results = cursor.fetchall()
    conn.close()
    
    return render_template_string(HTML_TEMPLATE, results=str(results))

# Command Injection Vulnerability
@app.route('/ping', methods=['POST'])
def ping():
    host = request.form['host']
    
    # VULNERABLE: Unsanitized input in system command
    try:
        result = subprocess.check_output(f"ping -c 1 {host}", shell=True, stderr=subprocess.STDOUT, text=True)
    except subprocess.CalledProcessError as e:
        result = e.output
    
    return render_template_string(HTML_TEMPLATE, results=result)

# XSS Vulnerability
@app.route('/comment', methods=['POST'])
def comment():
    user_comment = request.form['comment']
    
    # VULNERABLE: No output encoding
    return render_template_string(HTML_TEMPLATE, results=f"Your comment: {user_comment}")

# Path Traversal Vulnerability
@app.route('/readfile', methods=['POST'])
def read_file():
    filename = request.form['filename']
    
    # VULNERABLE: No path validation
    try:
        with open(filename, 'r') as f:
            content = f.read()
    except Exception as e:
        content = f"Error: {str(e)}"
    
    return render_template_string(HTML_TEMPLATE, results=content)

if __name__ == '__main__':
    init_db()
    print("Starting vulnerable app on http://localhost:5000")
    app.run(debug=True, host='0.0.0.0', port=5000)