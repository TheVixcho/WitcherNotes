import { initializeApp } from "firebase/app";
import { getAnalytics, isSupported } from "firebase/analytics";

// Configuración de Firebase para WitcherNotes
const firebaseConfig = {
  apiKey: import.meta.env.VITE_FIREBASE_API_KEY || "AIzaSyD-placeholder-key",
  authDomain: "witchernote.firebaseapp.com",
  projectId: "witchernote",
  storageBucket: "witchernote.firebasestorage.app",
  messagingSenderId: "883106333821",
  appId: "1:883106333821:web:37ab79fb59442c40b31d14"
};

// Inicializar Firebase
const app = initializeApp(firebaseConfig);

// Inicializar Analytics solo si el entorno lo soporta (en navegador)
let analytics = null;
if (typeof window !== "undefined") {
  isSupported().then(supported => {
    if (supported) {
      analytics = getAnalytics(app);
    }
  }).catch(() => {
    // Analytics opcional
  });
}

export { app, analytics };

