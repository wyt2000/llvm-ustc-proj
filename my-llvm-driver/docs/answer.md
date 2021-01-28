# 思考题第一题

```c
# Step 2 采用NCA方法,计算各个节点的直接支配节点
    for(i : [2,NextDFSNum-1] ){
		SDomNum = Node[i].Semi
        WIDomCandidate = Node[i].IDom	#初始为当前节点的父亲
        while (Node[WIDomCandidate].DFSNum > SDomNum)
        	WIDomCandidate = Node[WIDomCandidate].IDom
        
        Node[i].IDom = WIDomCandidate	
    }   
```
### 在runSemiNCA函数Step 2计算NCA时,如何证明找到的 WIDomCandidate一定是Node[i].Semi和Node[i].Parent的最近共同祖先？

#### 初步证明：
设每一个结点 $  v$ 在流图$ G$ 中的半支配结点为 $ sdom(v)$,在 $ G$  中的直接支配结点为 $ idom(v)$;设 $ G$ 经过 DFS 产生的生成树为 $ T$,结点 $ v$  在 $ T$  中的 DFS 编号为 $ DFSNum(v)$,在 $ T$  中的父节点为 $ parent_T(v)$;设产生的支配树为 $ D$,且 $ T$  和 $ D$ 中的根结点都是 $ r$.

首先给出三个引理：
**引理1**: *在流图 $ G$ 中,结点 $ u$ 支配结点 $ v$ 当且仅当在对应的支配树 $ D$ 中,结点 $ u$ 是结点 $ v$ 的祖先.*
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**证明：** 这里由支配树的定义可以显然得证.

**引理2**: *在流图 $ G$ 的 DFS 生成树 $ T$ 中, $ Sdom(v)$ 是 $ v$ 的祖先.*
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**证明：** 由于在 $ v$ 的半必经路径$ Sdom(v) \xrightarrow[\text{G}]{\text{}} v_1 \xrightarrow[\text{G}]{\text{}} v_2 \xrightarrow[\text{G}]{\text{}} v_k \xrightarrow[\text{G}]{\text{}} v$ 中,对 $ 1 \leq i \leq k$,有 $ DFSNum(v_i) > DFSNum(v) > DFSNum(Sdom(v))$. 则在该路径上，必存在某个节点 $w$ 为$Sdom(v)$ 和 $v$ 在 $T$ 中的公共祖先. 又因为$ DFSNum(w) \leq DFSNum(Sdom(v))$, 因此只能 $w = Sdom(v)$. 所以，$Sdom(v)$ 是 $v$ 在 $T$ 中的祖先.


**引理3**: *在流图 $ G$  中,如果结点 $ u$  支配结点 $ v$,那么必有 $ DFSNum(u) \leq DFSNum(v)$.*
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;**证明：** 假设结点 $ u$  支配结点 $ v$,却有 $ DFSNum(u) > DFSNum(v)$,说明结点 $ u$  在 $ v$  之后被 DFS 访问到.此时,在 $ G$  的生成树 $ T$  中存在路径 $ P: r \xrightarrow[\text{T}]{\text{*}} v$  且 $ P$  上任意结点 $ w$  都有 $ DFSNum(w) \leq DFSNum(v)$.因此,结点 $ u$  不在路径 $ P$  上.这说明结点 $ u$  不支配结点 $ v$,与假设矛盾.

由以上算法的 step2 可知, $ WIDomCandidate$  是从 $ parent_T(v)$  开始,不断在支配树 $ D$  上迭代寻找其父结点,直到找到第一个 $ WIDomCandidate$ 使得$ DFSNum(WIDomCandidate) \leq DFSNum(Sdom(v)) $.

显然,最后得到的 $ WIDomCandidate $ 是 $ parent_T(v) $ 在支配树 $ D $ 上的祖先.**下面证明 $ WIDomCandidate $ 也是 $ Sdom(v) $ 在支配树 $ D $ 上的祖先.**

(1) 若在流图 $ G $ 中, $ Sdom(v) $ 支配 $ parent_T(v) $.那么由引理1可知,在支配树 $ D $ 上, $ Sdom(v) $ 是 $ parent_T(v) $ 的祖先.因此在 $ D $ 中由 $ parent_T(v) $ 向祖先迭代寻找 $ WIDomCandidate $ 的过程中,最终会出现 $ DFSNum(WIDomCandidate) = DFSNum(Sdom(v)) $ 的情况.由于每个结点的 $ DFSNum $ 互异,因此 $ WIDomCandidate = Sdom(v) $.显然, $ WIDomCandidate $ 是 $ Sdom(v) $ 在 $ D $ 上的祖先.

(2) 若在流图 $ G $ 中, $ Sdom(v) $ 不支配 $ parent_T(v) $.那么由引理1可知, $ Sdom(v) $ 不是 $ parent_T(v) $ 在支配树 $ D $ 上的祖先.因此按照以上的算法最终找到的 $ WIDomCandidate \neq Sdom(v) $,因此必有 $ DFSNum(WIDomCandidate) < DFSNum(Sdom(v)) $.此时, $ WIDomCandidate $ 是 $ parent_T(v) $ 在支配树 $ D $ 上的祖先,因此 $ WIDomCandidate $ 在流图 $ G $ 中支配 $ parent_T(v) $.

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;假设 $ WIDomCandidate $ 在流图 $ G $ 中不支配 $ Sdom(v) $,则在 $ G $ 中存在路径 $ P_1: r \xrightarrow[\text{G}]{\text{*}} Sdom(v) $ 且 $ P_1 $ 不经过 $ WIDomCandidate $.又由引理2可知,在生成树 $ T $ 中 $ Sdom(v) $ 是 $ v $ 的祖先,又因为 $ Sdom(v) \neq v $,因此在生成树 $ T $ 中 $ Sdom(v) $ 是 $ parent_T(v) $ 的祖先,所以在 $ G $ 中 $ Sdom(v) $ 是 $ parent_T(v) $ 的祖先.由半必经结点的性质可知,对任意结点 $ a $ 在路径 $ P_2: Sdom(v) \xrightarrow[\text{G}]{\text{*}} a \xrightarrow[\text{G}]{\text{+}} parent_T(v) $ 中,都有 $ DFSNum(a) \geq DFSNum(Sdom(v)) $.又因为 $ DFSNum(WIDomCandidate) < DFSNum(Sdom(v)) $,所以 $ WIDomCandidate $ 不在路径 $ P_2 $ 上.

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;因此, $ G $ 中的路径 $ P_1 + P_2: r \xrightarrow[\text{G}]{\text{*}} Sdom(v) \xrightarrow[\text{G}]{\text{+}} parent_T(v) $ 中不含 $ WIDomCandidate $ 结点,这与 $ WIDomCandidate $ 支配 $ parent_T(v) $ 相矛盾.所以, $ WIDomCandidate $ 在流图 $ G $ 中仍然支配 $ Sdom(v) $.


综合(1)(2)可知, $ WIDomCandidate $ 是 $ Sdom(v) $ 在支配树 $ D $ 上的祖先.因此, $ WIDomCandidate $ 是 $ Sdom(v) $ 和 $ parent_T(v) $ 在支配树 $ D $ 上的共同祖先.**以下证明 $ WIDomCandidate $ 是 $ Sdom(v) $ 和 $ parent_T(v) $ 在支配树 $ D $ 上的最近共同祖先.**

假设 $ WIDomCandidate $ 不是 $ Sdom(v) $ 和 $ parent_T(v) $ 在支配树 $ D $ 上的最近共同祖先,那么存在 $ WIDomCandidate^\prime $ 为 $ Sdom(v) $ 和 $ parent_T(v) $ 在支配树 $ D $ 上的共同祖先且 $ WIDomCandidate^\prime $ 比 $ WIDomCandidate $ 在 $ D $ 上更接近 $ parent_T(v) $.此时 $ parent_T(v) $ 经过更少次数的迭代可以找到 $ WIDomCandidate^\prime $,说明 $ DFSNum(WIDomCandidate^\prime) > DFSNum(WIDomCandidate) $.又因为在 $ D $ 上, $ WIDomCandidate $ 是 $ parent_T(v) $ 的最近的一个 $ DFSNum $ 不大于 $ DFSNum(Sdom(v)) $ 的祖先,因此 $ DFSNum(WIDomCandidate^\prime) > Sdom(v) $.由引理3可知,在 $ G $ 中 $ WIDomCandidate^\prime $ 不能支配 $ Sdom(v) $,因此 $ WIDomCandidate^\prime $ 不是 $ Sdom(v) $ 在支配树 $ D $ 上的祖先,与假设矛盾.

综合以上证明可知,按照以上算法得到的 $ WIDomCandidate $ 是 $ Sdom(v) $ 和 $ parent_T(v) $ 在支配树 $ D $ 上的最近共同祖先.  $ \square $ 

# 循环统计分析
一个典型的循环由 cond, body, inc 三个基本块组成：在 cond 处判断是否满足循环条件，满足则进入 body，否则跳转到 inc 后面的基本块。对于 inc 基本块，它的后继基本块为 entry。
如果当前结点的后继结点支配当前结点，说明当前结点对应的基本块是 inc 基本块，即对应循环的出口。
如果当前结点的支配它的前驱结点，说明当前结点对应的基本块是 cond 基本块，即对应循环的入口。
所以只需要在循环的入口记录循环深度增加，在循环的出口输出循环信息即可。
但是这种方法无法处理死循环没有回边的问题，比如一个 for 循环内部是一个 while(1) ，则 for 循环的 inc 块没有前驱，它不支配 cond 块，for 循环没有回边。