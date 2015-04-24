(ns agv-server.state)

(def state (atom {}))
(def users (atom {}))
(def warehouse (atom [[ [:free] [:free] [:free] [:free] [:free] [:free] [:free] [:free]]
                      [ [:free] [:none] [:none] [:none] [:none] [:none] [:none] [:free]]
                      [ [:free] [:none] [:free] [:free] [:free] [:none] [:none] [:free]]
                      [ [:free] [:none] [:free] [:free] [:free] [:free] [:none] [:free]]
                      [ [:free] [:none] [:free] [:free] [:free] [:free] [:none] [:free]]
                      [ [:free] [:none] [:free] [:free] [:free] [:free] [:none] [:free]]
                      [ [:free] [:none] [:free] [:free] [:free] [:free] [:free] [:free]]
                      [ [:free] [:none] [:free] [:free] [:free] [:free] [:free] [:free]]
                      [ [:free] [:shelf] [:free] [:free] [:free] [:free] [:free] [:free]]
                      [ [:free] [:free] [:free] [:free] [:free] [:free] [:free] [:free]]]))


(defn add-client
  [client stream]
  (if-not (contains? @state client)
    (swap! state assoc client {:stream stream})))

(defn remove-client
  [client]
  (if (contains? @state client)
    (do
      (if-let [coords (get-in @state [client :ready])]
        (swap! warehouse assoc-in coords [:free]))
      (swap! state dissoc client))))

(defn get-client
  [client]
  (get @state client))

(defn get-clients
  []
  @state)

(defn get-any-client
  []
  (let [s @state
        readys (filter #(:ready (second %)) s)
        r (into {} readys)]
    (first r)))

(defn set-client-ready
  [client coords]
  (if-let [from (get-in @state [client :ready])]
    (swap! warehouse
           #(assoc-in (assoc-in % from [:free])
                      coords [:agv]))
    (swap! warehouse assoc-in coords [:agv]))
  (swap! state assoc-in [client :ready] coords))


(defn add-user
  [user stream]
  (swap! users assoc user {:stream stream}))

(defn remove-user
  [user]
  (swap! users dissoc user))

(defn get-users
  []
  @users)

(defn get-map
  []
  @warehouse)

