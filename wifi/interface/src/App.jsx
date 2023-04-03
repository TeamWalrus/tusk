import { Link, Route } from "wouter";
import NavBar from "./components/NavBar"
import Home from "./views/home"
import Settings from "./views/settings"
import './App.css'

function App() {
  return (
    <div className="App">
      <NavBar />
      <Route path="/">
          <Home />
        </Route>
      <Route path="/settings">
        <Settings />
      </Route>
    </div>
  )
}

export default App
