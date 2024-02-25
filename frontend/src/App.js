import "./App.css";
import React, { useEffect, useState } from "react";

function App() {

  const [sensorValue, setSensorValue] = useState([]);

  // Use useEffect hook to establish WebSocket connection and handle incoming messages.
  useEffect(() => {
    const ws = new WebSocket("ws://192.168.1.168/ws");

    ws.onopen = () => {
      console.log("Connected to ESP32!");
    };

    ws.onmessage = (event) => {

      const data = JSON.parse(event.data);
      setSensorValue(data);
    };

    ws.onclose = () => {
      console.log("Disconnected from ESP32");
    };

    return () => {
      ws.close();
    };
  }, []);

  return (
    <div className="App">
      <div>
        {/* Display the sensorValue state in your JSX. */}
        <h1>Temperature: {sensorValue.temperature}</h1>
        <h1>Humidity: {sensorValue.humidity}</h1>
      </div>
    </div>
  );
}

export default App;
