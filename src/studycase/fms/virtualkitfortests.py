from flask import Flask, request
import time
import logging

app = Flask(__name__)

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

@app.route('/passadeiras', methods=['GET'])
def passadeiras():
    skill = request.args.get('skill')
    if not skill:
        return "Missing skill parameter", 400
    
    logging.info(f"Received request for passadeiras (skill: {skill})")
    time.sleep(5)
    logging.info(f"Finished request for passadeiras (skill: {skill})")
    return "OK", 200

@app.route('/estacao', methods=['GET'])
def estacao():
    skill = request.args.get('skill')
    if not skill:
        return "Missing skill parameter", 400
    
    logging.info(f"Received request for estacao (skill: {skill})")
    time.sleep(5)
    logging.info(f"Finished request for estacao (skill: {skill})")
    return "OK", 200

if __name__ == '__main__':
    logging.info("Starting Virtual Kit Mock Server on port 80...")
    try:
        app.run(host='0.0.0.0', port=80)
    except PermissionError:
        logging.error("Permission denied to run on port 80. Trying port 8080...")
        app.run(host='0.0.0.0', port=8080)
