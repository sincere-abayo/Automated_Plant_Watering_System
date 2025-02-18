from flask import Flask, request, jsonify, render_template
from datetime import datetime, timedelta

import mysql.connector

app = Flask(__name__)

# MySQL connection
db = mysql.connector.connect(
    host="localhost",
    user="root",
    password="root",
    database="plant_system",
    charset="utf8mb4",
    collation="utf8mb4_general_ci"
)

cursor = db.cursor()

@app.route('/')
def index():
    cursor.execute("SELECT * FROM moisture_data ORDER BY timestamp DESC LIMIT 10")
    readings = cursor.fetchall()
    stats = get_consumption_stats()
    return render_template('index.html', readings=readings, stats=stats)



def get_consumption_stats():
    now = datetime.now()
    stats = {}
    
    # Daily consumption
    cursor.execute("""
        SELECT COUNT(*) FROM moisture_data 
        WHERE timestamp >= CURDATE()
    """)
    stats['daily'] = cursor.fetchone()[0]
    
    # Weekly consumption
    cursor.execute("""
        SELECT COUNT(*) FROM moisture_data 
        WHERE timestamp >= DATE_SUB(CURDATE(), INTERVAL 7 DAY)
    """)
    stats['weekly'] = cursor.fetchone()[0]
    
    # Monthly consumption
    cursor.execute("""
        SELECT COUNT(*) FROM moisture_data 
        WHERE timestamp >= DATE_SUB(CURDATE(), INTERVAL 30 DAY)
    """)
    stats['monthly'] = cursor.fetchone()[0]
    
    # Yearly consumption
    cursor.execute("""
        SELECT COUNT(*) FROM moisture_data 
        WHERE timestamp >= DATE_SUB(CURDATE(), INTERVAL 1 YEAR)
    """)
    stats['yearly'] = cursor.fetchone()[0]
    
    return stats

@app.route('/save_data', methods=['GET'])
def save_data():
    moisture = request.args.get('moisture')
    
    if moisture:
        cursor.execute("INSERT INTO moisture_data (moisture_level) VALUES (%s)", (moisture,))
        db.commit()
        return jsonify({"status": "success", "message": "Data stored successfully!"})
    
    return jsonify({"status": "error", "message": "Missing data!"})

@app.route('/get_data', methods=['GET'])
def get_data():
    cursor.execute("SELECT * FROM moisture_data ORDER BY timestamp DESC LIMIT 10")
    data = cursor.fetchall()
    return jsonify(data)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
