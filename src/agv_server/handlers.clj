(ns agv-server.handlers
  (:require [agv-server.state :as st]
            [manifold.stream :as s]))


(defmacro with-session
  [session & body]
  `(if-let [c# (:client ~session)]
     (do ~@body)
     (str "ERROR Not authenticated")))

(defn login-handler
  [[client & params] session]
  (if (:client @session)
    (str "ERROR You are already authenticated")
    (if-let [status (st/add-client client (:stream @session))]
      (do (swap! session assoc :client client)
          (str "OK"))
      (str "ERROR Client already exist"))))

(defn pong-handler
  [session]
  (with-session @session
    (swap! session assoc :pong true)
    (str "OK")))

(defn ready-handler
  [[coords & params] session]
  (with-session @session
    (st/set-client-ready (:client @session))
    (str "OK")))

(defn ok-handler
  [session]
  (with-session @session nil))

(defn err-handler
  [command params session]
  (str "ERROR Unknown command, session: " @session))

(defn protocol-handler
  [msg session]
  (let [[command & params] (clojure.string/split msg #"\s+")]
    (case command
      "LOGIN" (login-handler params session)
      "PONG" (pong-handler session)
      "READY" (ready-handler params session)
      "OK" (ok-handler session)
      (err-handler command params session))))
