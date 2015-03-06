(ns agv-server.protocol
  (:require [aleph.tcp :as tcp]
            [manifold.stream :as s]
            [manifold.deferred :as d]
            [gloss.core :as g :refer [defcodec]]
            [gloss.io :as io]
            [agv-server.handlers :refer [protocol-handler]]
            [agv-server.state :as st]))

(defcodec protocol (g/string :utf-8 :delimiters ["\r\n"]))

(defn wrap-duplex-stream
  [protocol s]
  (let [out (s/stream)]
    (s/connect (s/map #(io/encode protocol %) out) s)
    (s/splice out (io/decode-stream s protocol))))

(defn start-server
  [handler port]
  (tcp/start-server
    (fn [s info]
      (handler (wrap-duplex-stream protocol s) info))
    {:port port}))

(defn handler
  [s info]
  (let [session (atom {})]
    (s/on-closed s #(some-> (:client @session) (st/remove-client)))
    (swap! session assoc :pong true)
    (s/connect (s/map #(protocol-handler % session) s) s)
    (d/loop []
      (Thread/sleep 5000)
      (if (:pong @session)
        (do 
          @(s/put! s "PING")
          (swap! session assoc :pong false)
          (d/recur))
        ; PING TIMEOUT
        (do
          @(s/put! s "ERROR Ping timeout")
          (prn "Ping timeout " info)
          (s/close! s))))))

