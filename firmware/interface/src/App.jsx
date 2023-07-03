import { Link, Route } from "wouter";
import NavBar from "./components/NavBar";
import Home from "./views/Home";
import Settings from "./views/Settings";
import { DeviceSettingsProvider } from "./components/DeviceSettingsProvider";
import "./App.css";

function App() {
  return (
    <div className="App">
      <DeviceSettingsProvider>
        <NavBar />
      </DeviceSettingsProvider>
      <Route path="/">
        <Home />
      </Route>
      <Route path="/settings">
        <Settings />
      </Route>
    </div>
  );
}

export default App;
