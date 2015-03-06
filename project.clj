(defproject agv-server "0.1.0-SNAPSHOT"
  :description "Server for AGV control"
  :url ""
  :license {:name ""
            :url ""}
  :dependencies [[org.clojure/clojure "1.6.0"]
                 [org.clojure/core.async  "0.1.346.0-17112a-alpha"]
                 [aleph  "0.4.0-beta3"]
                 [gloss "0.2.4"]]
  :main ^:skip-aot agv-server.core
  :target-path "target/%s"
  :profiles {:uberjar {:aot :all}})
