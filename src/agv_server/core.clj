(ns agv-server.core
  (:require [aleph.tcp :as tcp]
            [manifold.stream :as s]
            [gloss.core :as g :refer [defcodec]]
            [gloss.io :as io]
            [agv-server.protocol :as p]
            [agv-server.http :as h])
  (:gen-class))



(defn -main
  [& args]
  (p/start-server p/handler 3001)
  (h/start-server h/app 3000))
