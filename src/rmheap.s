// Randomized Meldable Heap 
type rmh_node key value parent left right

rmh_merge H1 H2 =
| less H1: leave H2
| less H2: leave H1
| when H2.key < H1.key: leave: rmh_merge H2 H1
| if 1 //1.rand
  then | H1.left <= rmh_merge H1.left H2
       | when H1.left: H1.left.parent <= H1
  else | H1.right <= rmh_merge H1.right H2
       | when H1.right: H1.right.parent <= H1
| H1

rmh_remove R = rmh_merge R.left R.right

rmh_pop_lowest N =
| when N.left: leave: rmh_pop_lowest N.left
| when N.parent: N.parent.left <= N.right
| N

rmh_pop_highest N =
| when N.right: leave: rmh_pop_highest N.right
| when N.parent: N.parent.right <= N.left
| N

type rmheap root size

rmheap.add Key Value =
| U = rmh_node
| U.key <= Key
| U.value <= Value
| $root <= rmh_merge U $root
| $root.parent <= 0
| !$size+1
| Me

rmheap.pop_lowest =
| less $size: bad 'cant pop empty heap'
| !$size-1
| rmh_pop_lowest $root

rmheap.pop_highest =
| less $size: bad 'cant pop empty heap'
| !$size-1
| rmh_pop_highest $root

export rmheap
