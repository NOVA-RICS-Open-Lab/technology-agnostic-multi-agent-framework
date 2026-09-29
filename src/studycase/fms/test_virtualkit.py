import pytest
from virtualkitfortests import app
import time

@pytest.fixture
def client():
    app.config['TESTING'] = True
    with app.test_client() as client:
        yield client

def test_passadeiras_endpoint(client):
    start_time = time.time()
    response = client.get('/passadeiras?skill=AB')
    end_time = time.time()
    
    assert response.status_code == 200
    assert b"OK" in response.data
    # Use a slightly lower threshold to account for processing time variability
    assert (end_time - start_time) >= 4.9

def test_estacao_endpoint(client):
    start_time = time.time()
    response = client.get('/estacao?skill=0')
    end_time = time.time()
    
    assert response.status_code == 200
    assert b"OK" in response.data
    assert (end_time - start_time) >= 4.9

def test_missing_skill_parameter(client):
    response = client.get('/passadeiras')
    assert response.status_code == 400
