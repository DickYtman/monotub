from flask import Flask, request, jsonify
from flask_mysqldb import MySQL

app = Flask(__name__)

# MySQL configurations
app.config['MYSQL_HOST'] = 'localhost'
app.config['MYSQL_USER'] = 'root'
app.config['MYSQL_PASSWORD'] = '12332156aaAA$'
app.config['MYSQL_DB'] = 'monotubdb'

mysql = MySQL(app)

@app.route('/api/readings', methods=['POST'])
def post_readings():
    content = request.json
    temperature = content['temperature']
    humidity = content['humidity']

    cur = mysql.connection.cursor()
    cur.execute("INSERT INTO readings (temperature, humidity) VALUES (%s, %s)", (temperature, humidity))
    mysql.connection.commit()
    cur.close()

    return jsonify({"message": "Data stored successfully!"}), 201

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0')


