(defproject agv-server "0.1.0-SNAPSHOT"
  :description "Server for AGV control"
  :url ""
  :license {:name ""
            :url ""}
  :dependencies [[org.clojure/clojure "1.6.0"]
                 [org.clojure/core.async "0.1.346.0-17112a-alpha"]
                 [aleph "0.4.0-beta3"]
                 [manifold "0.1.0-SNAPSHOT"]
                 [gloss "0.2.4"]
                 [compojure "1.3.2"]
                 [org.clojure/clojurescript "0.0-3126"]]
  :main ^:skip-aot agv-server.core
  :target-path "target/%s"
  :profiles {:uberjar {:aot :all}}
  :plugins [[lein-cljsbuild "1.0.5"]]
  :cljsbuild {
    :builds [{
      :source-paths ["src/agv_server/cljs"]
      :compiler {
        :main "agv-server.cljs.core"
        :output-to "resources/public/js/main.js"
        :output-dir "resources/public/js"
        :source-map "resources/public/js/main.js.map"
        :optimizations :whitespace
        :pretty-print true}}]})
