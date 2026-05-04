import { Route } from "wouter";
import NavBar from "./components/NavBar";
import Home from "./views/Home";
import Settings from "./views/Settings";
import { DeviceSettingsProvider } from "./components/DeviceSettingsProvider";
import { UIProvider } from "./components/UIProvider";
import Toast from "./components/Toast";
import "./App.css";

export default function App() {
  return (
    <div className="App">
      <UIProvider>
        <DeviceSettingsProvider>
          <NavBar />
          <Toast />
          <Route path="/" component={Home} />
          <Route path="/settings" component={Settings} />
        </DeviceSettingsProvider>
      </UIProvider>
    </div>
  );
}
