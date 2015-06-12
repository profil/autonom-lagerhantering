(ns agv-server.state
  (:require [agv-server.pathfinding :as p]
            [manifold.stream :as s]))

(def state (atom {}))
(def users (atom {}))
(def warehouse (atom [[ :s-0  :free :free    :s-1  ]
                      [ :free :free :free    :free ]
                      [ :free :free :free    :free ]
                      [ :free :free :station :free ]]))
(def orders (atom {}))
(def shelves (atom {:s-0 {:coords [0 0] :agv nil}
                    :s-1 {:coords [0 3] :agv nil}
                    :station {:coords [3 2]}}))
(def inventory (atom {"10000" :s-0
                      "10001" :s-1
                      "10002" :s-0
                      "10003" :s-0}))


(defn add-client
  [client stream]
  (if-not (contains? @state client)
    (swap! state assoc client {:stream stream})))

(defn remove-client
  [client]
  (if (contains? @state client)
    (swap! state dissoc client)))

(defn get-client
  [client]
  (get @state client))

(defn get-clients
  []
  @state)

(defn get-any-client
  []
  (let [s @state
        readys (filter #(and (not (:busy (second %))) (:ready (second %))) s)
        r (into {} readys)]
    (first r)))

(declare agv-go-to)


(defn do-next-order
  [agv]
  (when-let [[id value] (first (filter #(-> % val :agv nil?) @orders))]
    ;; TODO: take all orders on the same shelf here!
    (swap! orders assoc-in [id :agv] agv)
    (let [shelf (get-in @orders [id :shelf])
          to (get-in @shelves [shelf :coords])]
      (swap! state assoc-in [agv :busy] to)
      (agv-go-to agv to))))

(defn set-client-ready
  [client coords]
  (swap! state assoc-in [client :ready] coords)
  (if-let [to (get-in @state [client :busy])]
    ; Continue with the task
    (agv-go-to client to)
    ; Work with new task!
    (do-next-order client)))


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
  (let [warehouse @warehouse
        agv-coords (keep #(-> % val :ready) @state)]
    (reduce
      (fn [map coords]
        (assoc-in map coords :agv))
      warehouse
      agv-coords)))

(defn get-orders
  []
  @orders)

(defn orders-done
  [orders shelf]
  (let [ids (keep #(when (-> % val :shelf (= shelf)) (key %)) orders)]
    (reduce
      (fn [map id]
        (assoc-in map [id :done] true))
      orders
      ids)))

(defn agv-lift
  [agv shelf]
  (swap! shelves assoc-in [shelf :agv] agv)
  (swap! state assoc-in [agv :lift] shelf)
  "LIFT")

(defn agv-lower
  [agv]
  (when-let [shelf (get-in @state [agv :lift])]
    (swap! shelves assoc-in [shelf :agv] nil)
    (swap! state assoc-in [agv :lift] nil)
    "LOWER"))

(defn lift-or-lower
  [agv shelf]
  (if (get-in @state [agv :lift])
    (agv-lower agv)
    (agv-lift agv shelf)))

(defn lift-or-lower-ok
  [agv]
  (if-let [shelf (get-in @state [agv :lift])]
    ; Lift is OK, now go to station
    (let [station-coords (get-in @shelves [:station :coords])]
      (swap! state assoc-in [agv :busy] station-coords)
      (agv-go-to agv station-coords))
    ; Lower is OK
    (do (swap! state assoc-in [agv :busy] nil)
        (do-next-order agv))))

(defn agv-go-to
  [agv to]
  (let [from (get-in @state [agv :ready])
        path (p/astar (get-map) from to)
        [n s & _] (p/directions path)
        stream (get-in @state [agv :stream])
        cell (get-in @warehouse to)]
    (println (-> cell name (.split "-") first))
    (if-not (nil? n)
      (if (= n s)
        (if (= (-> cell name (.split "-") first) "s")
          (str n " 1")
          (str n " 2"))
        (str n " 1"))
      (case (-> cell name (.split "-") first)
        "s" (lift-or-lower agv cell)
        "station" (swap! orders orders-done (get-in @state [agv :lift]))
        "ERROR Unknown error"))))


(defn get-part
  [id]
  (if-let [shelf (get @inventory id)]
    (let [agv (first (keep #(if (-> % val :shelf (= shelf)) (-> % val :agv)) @orders))]
      (if agv
        (pr-str [:orders (swap! orders assoc id {:agv agv :done false :shelf shelf})])
        (let [agv (first (get-any-client))]
          (when-not (nil? agv)
            (swap! state assoc-in [agv :busy] (get-in @shelves [shelf :coords]))
            (s/put! (get-in @state [agv :stream])
                    (agv-go-to agv (get-in @shelves [shelf :coords]))))
          (pr-str [:orders (swap! orders assoc id {:agv agv :done false :shelf shelf})]))))
    (pr-str [:error "Ogiltigt artikelnummer"])))

(defn return-shelf
  [agv shelf]
  (let [to (get-in @shelves [shelf :coords])]
    (swap! state assoc-in [agv :busy] to)))

(defn accept-order
  [id]
  (let [shelf (get-in @orders [id :shelf])
        agv (get-in @orders [id :agv])
        new-orders (swap! orders dissoc id)]
    (when-not (some #(-> % val :shelf (= shelf)) new-orders)
      ;; Return shelf
      (return-shelf agv shelf))
    (pr-str [:orders new-orders])))

(defn abort-order
  [id]
  ;; Take care of putting back shelf if holding one
  (let [order (get @orders id)
        shelf (get order :shelf)
        agv (get order :agv)]
    (if (get-in @state [agv :lift])
      (return-shelf agv shelf)
      (swap! state assoc-in [agv :busy] nil))
    (pr-str [:orders (swap! orders dissoc id)])))

(defn blocked-map
  [agv direction]
  (let [p (get-in @state [agv :ready])
        dirs {"SOUTH" [ 1  0]
              "NORTH" [-1  0]
              "EAST"  [ 0  1]
              "WEST"  [ 0 -1]}
        coords (vec (map #(+ %1 %2) p (dirs direction)))]
    (swap! warehouse assoc-in coords :block)
    (get-map)))

