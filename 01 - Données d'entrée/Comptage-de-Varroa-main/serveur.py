from flask import Flask, request, render_template
import os
import subprocess
import sqlite3
from datetime import datetime

app = Flask(__name__)

# Configuration
IMAGE_DIR = "./images/"
os.makedirs(IMAGE_DIR, exist_ok=True)

# Initialisation BDD
def init_db():
    conn = sqlite3.connect('varroa.db')
    cursor = conn.cursor()
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS counts (
            id INTEGER PRIMARY KEY,
            date TEXT NOT NULL,
            varroa_count INTEGER NOT NULL,
            image_path TEXT NOT NULL
        )
    ''')
    conn.commit()
    return conn

@app.route('/upload', methods=['POST'])
def upload():
    try:
        # Sauvegarde de l'image
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"varroa_{timestamp}.jpg"
        filepath = os.path.join(IMAGE_DIR, filename)
        
        with open(filepath, 'wb') as f:
            f.write(request.data)

        # Lance l'analyse en arrière-plan
        subprocess.Popen(["python", "analyse_varroas.py", filepath])
        
        return f"Image {filename} reçue. Analyse en cours...", 200

    except Exception as e:
        return f"Erreur: {str(e)}", 500

@app.route('/dashboard')
def dashboard():
    conn = init_db()
    counts = conn.execute("SELECT * FROM counts ORDER BY date DESC").fetchall()
    conn.close()
    return render_template('dashboard.html', counts=counts)

if __name__ == '__main__':
    init_db()
    app.run(host="0.0.0.0", port=5000, debug=True)