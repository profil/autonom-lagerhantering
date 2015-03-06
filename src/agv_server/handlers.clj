(ns agv-server.handlers
  (:require [agv-server.state :as st]
            [manifold.stream :as s]))

(defn valid-session?
  [session]
  (:client session))

(defn login-handler
  [[client & params] session]
  (if (:client @session)
    (str "ERROR You are already authenticated")
    (if-let [status (st/add-client client)]
      (do (swap! session assoc :client client)
          (str "OK"))
      (str "ERROR Client already exist"))))

(defn pong-handler
  [session]
  (if (valid-session? @session)
    (do
      (swap! session assoc :pong true)
      (str "OK"))
    (str "ERROR Not authenticated")))

(defn err-handler
  [command params session]
  (str "ERROR Unknown command, session: " @session))

(defn protocol-handler
  [msg session]
  (let [[command & params] (clojure.string/split msg #"\s+")]
    (case command
      "LOGIN" (login-handler params session)
      "PONG" (pong-handler session)
      (err-handler command params session))))
