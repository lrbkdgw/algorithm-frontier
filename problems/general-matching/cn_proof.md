# 通过持久化整数载体与归一化路由实现确定性精确一般图匹配

**icekylinx · GPT-6 Astra Pro**

**定理与证明稿。** 本文构造一种确定性精确匹配算法，其额外开销为对数项的五分之四次幂；构造中使用第 2.2 节列出的来源结果，以及第 11.6 定理汇总的局部构造。完整数学构造与当前的[多项式时间参考实现](reference/README.md)具有不同的实现覆盖范围。

## 摘要

我们针对任意无向图构造确定性算法，精确求解最大基数匹配。算法围绕两个接口展开：一是对实际整数匹配账本的持久化表示；二是具有明确遍历与生命周期保证的动态最小比率环流算法。构造采用载体表示及其局部暴露过程；在不假设存在完美匹配的前提下建立最优值差距表述；从守恒的近可行流中精确恢复投影；并结合顺序路由与归一化稀疏化，分别界定结构参数和构造工作量。第 11.6 定理将局部原始提升、修复、出现项、退役以及有限精度算术的证明组合为所需的环搜索算法。完整组合给出一个确定性程序，其对数开销指数为 <code class="math">4/5</code>，且只在所声明的接口处调用列名的来源结果。所有返回边均保留原始输入标识。仓库另行实现并测试了一个多项式时间精确求解器；它尚未实现完整的快速层次结构。

## 1. 结果、贡献与证明范围

设无向图 <code class="math">G</code> 由 <code class="math">n</code> 个顶点和 <code class="math">m</code> 条边记录给定。输入允许自环和平行边。记 <code class="math">m_s</code> 为不同非自环边的数量，记 <code class="math">\nu(G)</code> 为最大匹配的基数。

**定理 1.1（确定性精确匹配）。** 在第 2.1 节的计算模型下，存在一个确定性程序，能够返回 <code class="math">G</code> 的显式最大基数匹配，并保留原始边标识；其运行时间为

<!-- math-display: 3dfffb10bdf03999a8ca90be
O(n+m)+(m_s+1)
 \\exp\\!\\left[O\\!\\left(
   (\\log(m_s+2))^{4/5}
   (\\log\\log(m_s+4))^{1/5}
 \\right)\\right].
 \\tag{1}
-->
![公式](../../assets/math/3dfffb10bdf03999a8ca90be.svg)

常数与输入无关。图可以不连通、非二分、顶点数为奇数，或不存在完美匹配。若采用第 3.1 节的精确预处理核，第二项中的 <code class="math">m_s</code> 可替换为实际边数 <code class="math">m_H\leq\min\{m_s,O(\nu(G)^2)\}</code>。输入只扫描一次。

核心贡献如下。

1. **实际整数载体。** 将每个整数账本维护为无环运输载体，并保留值为零的载体边。若一个批次触及 <code class="math">q</code> 个关联项，只需 <code class="math">O(q)</code> 次森林枢轴操作。在底层账本达到稳定状态时，载体会诱导出若干所选树，其箱权重小于 <code class="math">2L</code>，并带有隐式的正向与逆向行指派。
2. **局部确定性暴露与任意最优差距。** 输出敏感的紧集发现和局部森林交换提供所需的结构操作。一个比当前匹配多出 <code class="math">K_{\mathrm g}</code> 条边的比较匹配，提供阶段证明所需的增广前缀。因此，在任意一般图上都能驱动精确的差距递减过程。
3. **精确投影与直接数值参数。** 若守恒有理流对整数界的总违反量小于 1，则可将其舍入为精确可行流。真实流与乐观割的恢复给出完整的残余剥离过程。二进制账本周期与独立选取的数值精度，使客户端总工作量为 <code class="math">S E^4\mathrm{poly}(\log S)</code>。
4. **顺序路由与归一化环境稀疏化。** 给出明确的有限局部过程、只有一个物理根映射的虚拟层次结构，以及散布—会合—反向的查询归纳。静态提供器的结构质量满足 <code class="math">\log\Gamma=O((p+r)\log U)</code>，而较大的构造因子仍计入局部工作。
5. **完整的定量组合。** 计入所选边的递归、延后兑现的初始更新额度、出现项池、有限精度算术，以及完整的递归质量乘积。将这些因素同时平衡后得到式 (1)。

定理使用第 2.2 节列出的来源结果。第 11.6 定理通过组合局部构造证明，提供所需的环搜索接口；随后第 12 节平衡这些构造的实际成本。本文并未声称某一篇来源论文单独证明了改进后的界。[检查点来源图](sources.md#checkpoint-provenance)记录了修订版 10 月 24 日检查点中已有的构造链。缺少端到端快速实现属于代码覆盖范围的限制，不是额外的数学假设。输出验证可以确认已接受匹配的最优性；更快的运行时间则由构造证明保证。

### 1.1 无条件可执行的配套实现

[标准库参考求解器](reference/README.md)实现了第 3.1 节的规范化、连通分量分解和精确边核。较早检查点中的[外端点压缩](reference/cover-compression-proof.md)保留计算所得顶点覆盖中的每一个可匹配子集，并使最终精确匹配搜索涉及的顶点数至多为 <code class="math">3\nu(G_i)</code>。跨边压缩与最终二分图求解均使用 Hopcroft–Karp；其他最终求解使用 Edmonds 经典花算法。对于非空的规范化输入分量 <code class="math">G_i</code>，运行时间为

<!-- math-display: 6c0fddf87e29fb75109bfdec
O\\!\\left(n+m\\log(m+2)+\\sum_i\\nu(G_i)^{p_i}\\right),
\\qquad p_i\\in\\{5/2,3\\}.
-->
![公式](../../assets/math/6c0fddf87e29fb75109bfdec.svg)

若第一个保留边核是二分图，则指数为 `5/2`；否则为 `3`。[完整二分图证明](reference/bipartite-proof.md)推导了按匹配数计的阶段界和线性时间证书提取；所有原本为二分图的输入分量都满足其条件。

求解器返回所选边的原始 ID，使用 <code class="math">O(n+m)</code> 条记录；实际标签字节数和比较成本另行计费。可选的证书模式也满足相同界。证书提取仍在原始边核上执行：若该核有 <code class="math">k_i</code> 个顶点、<code class="math">h_i</code> 条边，则二分图情形耗时 <code class="math">O(k_i+h_i)</code>，其他情形耗时 <code class="math">O(h_i+k_i(\nu(G_i)+1))</code>，均包含在相应求解界内。输入不需要提供匹配数估计。[单独的验证器](reference/verify_certificate.py)通过图扫描和比较排序检查边核保持精确性，并核验 Tutte–Berge 上界；它不调用匹配算法。参考实现文档证明，验证通过即可认证最大基数。其[自包含的提取证明](reference/certificate-proof.md)从一棵已穷尽的交替森林中导出紧的一般图奇分量障碍；二分图证明则通过最小顶点覆盖得到相同的证书等式。该实现采用经典匹配搜索，不执行第 11.6 定理的环搜索层次结构。额外的多项式匹配工作仅用于这一可执行配套实现，不计入也不构成快速组合定理的前提。

## 2. 模型、记号与继承的基本工具

### 2.1 计算与标识约定

采用字长足以容纳输入顶点或边标识的 word RAM。输入顶点名是 <code class="math">[n]</code> 中的整数，因此对端点对进行计数排序／基数排序以及压缩可在 <code class="math">O(n+m)</code> 时间完成。更大的整数及有理数运算均按实际字操作计费。一个机器字绝不存放未计费的任意长整数。

除熵或指数势明确使用自然对数外，所有对数均以 2 为底。记号 <code class="math">\mathrm{poly}(U)</code> 始终表示次数固定的多项式。下文所有 <code class="math">O(\cdot)</code> 中的常数，对引入的所有增长参数均一致。

*生命周期标识符*命名一个顶点、边或已暴露出现项从创建到退役的单次实例。复用一个地址即产生新的生命周期。平行边具有不同标识符。*当前图*（immediate graph）是路径当前所定义于其中的图；即使两个不同的当前顶点之后投影到同一个物理顶点，它们仍被视为不同顶点。

对于游走 <code class="math">P</code>，原始长度按每次遍历计数。对于环境路径表 <code class="math">\Pi</code>，<code class="math">L(\Pi)</code> 表示最大路径长度，<code class="math">C(\Pi)</code> 表示最大顶点出现拥塞度（包括端点）。共享端点可按固定因子重复计数。空路径仍保留端点注记。

全文始终区分以下参数。

| 符号 | 含义 |
|---|---|
| <code class="math">S=2^s</code> | 次三次匹配问题的主规模 |
| <code class="math">U=2s</code> | 数值实例规模至多为 <code class="math">S^2</code> 时的主对数规模 |
| <code class="math">h=\lceil\log(U+4)\rceil</code> | 对数辅助因子 |
| <code class="math">K_{\mathrm g}</code> | 一个阶段中的目标匹配改进量 |
| <code class="math">L</code> | 每行的整数分配量 |
| <code class="math">p,r</code> | 路由与度数分带分解参数 |
| <code class="math">K_{\mathrm r}</code> | 触及集合修复层数 |
| <code class="math">k=2^x</code> | 递归规模缩减因子 |
| <code class="math">\Gamma,F</code> | 静态结构质量与构造工作因子 |
| <code class="math">P_{\mathrm d}</code> | 局部投影中的父账本债务 |
| <code class="math">P_{\mathrm b}</code> | 编码子记录及递推因子 |
| <code class="math">E,D,q</code> | 后端上限、取整后的数值上限与数值精度 |

### 2.2 外部输入

我们在下表所示接口处使用以下列名来源结果。参考文献给出主要来源位置。这些是通常意义上的数学依赖；参数化组合与额外算法接口将在下文及链接的配套证明中给出。

| 输入 | 所用接口 |
|---|---|
| **[OAI]，第 3–5 节** | 确定性的行／箱状态恒等式、对数描述符、有界目标值与正性服务，以及局部交换网络的探索／锁定不变量。第 4–8 节将说明本文的表示、差距、数值参数和精确恢复。 |
| **[AHLT]** | 确定性最坏情形动态森林，支持 link、cut、expose、路径极值、路径惰性加法和根化祖先查询；允许内部度数约简。 |
| **[CHHIP]，引理 3.5** | 在工作量与给定探索预算成比例的情况下，发现一条有向出边；并给出规定的有界体积输出或不存在结论。 |
| **[KMPG]、[CKL] 与 [AN]** | 确定性中心／稳定森林和原始边加权树几何，包括逐点固定伸展标签和单调森林割。第 11.5 节说明本文组合所需的有限位接口。 |
| **[CGLNPS]、[CS21]** | 确定性平衡割与扩张子剪枝基本工具。第 10 节将它们组合为度数分带接口，并确保每次调用均处于其实际规模参数范围。 |
| **[CS]、[CHS] 与 [KMPS]** | 组合层次结构、洗牌器以及到环距离几何；使用第 9、11 节描述的局部接口。顺序工作、查询、归一化与原始提升的规格由本文另行给出。 |
| **[Gabow]** | 从任意匹配出发的确定性增广路搜索，包括搜索穷尽时的返回结果，工作量近线性。 |

匹配状态的局部化输入有一个特别有用的精确形式。将底数为 <code class="math">c+1</code>、债务为 <code class="math">P_{\mathrm d}</code> 的整数父状态投影到底数为 <code class="math">c</code> 的状态时，使用冻结的父状态。其阈值网络暴露 <code class="math">O(P_{\mathrm d}+1)</code> 条记录，保留永久汇点锁，源点总亏缺不超过 <code class="math">P_{\mathrm d}</code>，并允许按第 8 节恢复真实区域／乐观区域。我们从来源框架继承这种局部性及其有界目标值／锁定记账；仅证明精确流存在，并不能证明其局部性。

定理依赖这些来源结果各自声明的适用范围。路由、归一化、生命周期和参数界由本文局部构造给出，不会将它们错误归因于接口或参数范围不同的来源结果。

## 3. 精确输入约简

### 3.1 清理与可选的二次边数核

去除自环；对于每一对无序非自环端点，仅保留一个原始标识；再删除孤立顶点。若所得图为空，则立即返回。令剩余简单图有 <code class="math">v_0\leq2m_s</code> 个顶点。

对于可选的边核，先取一个贪心极大匹配 <code class="math">M_0</code>，令 <code class="math">C=V(M_0)</code>、<code class="math">c=&#124;C&#124;</code>，并令 <code class="math">I=V(G)\setminus C</code>。由极大性可知 <code class="math">I</code> 是独立集。保留所有 <code class="math">C</code> 内部的边；对每个 <code class="math">u\in C</code>，按原始边顺序，在 <code class="math">I</code> 中保留恰好 <code class="math">\min\{c,&#124;N(u)\cap I&#124;\}</code> 个不同邻居。删除所得保留图 <code class="math">H</code> 中的孤立顶点。该构造及引理 3.1 与数值后端无关。

**引理 3.1。** 该边核可在线性时间内构造，并满足

<!-- math-display: 8f781f2b56a2c9b665bf750a
\\nu(H)=\\nu(G),\\qquad
 m_H\\leq\\min\\left\\{m_s,\\binom c2+c^2\\right\\},\\qquad
 c\\leq2\\nu(G).
 \\tag{2}
-->
![公式](../../assets/math/8f781f2b56a2c9b665bf750a.svg)

**证明。** 极大匹配的端点构成顶点覆盖，因此任何匹配至多有 <code class="math">c</code> 条边。若某个最大匹配使用了被删去的边 <code class="math">uv</code>，其中 <code class="math">u\in C</code>、<code class="math">v\in I</code>，则 <code class="math">u</code> 在 <code class="math">I</code> 中有 <code class="math">c</code> 个保留邻居。暂时移除 <code class="math">uv</code>。此时 <code class="math">I</code> 中至多有 <code class="math">c-1</code> 个顶点被匹配，所以这些保留邻居中至少有一个空闲。将 <code class="math">u</code> 改与该邻居匹配。每次替换都会减少所用删边的数目，同时保持匹配基数。因此 <code class="math">H</code> 中存在一个最大匹配。反向不等式由 <code class="math">H\subseteq G</code> 得出。最后，<code class="math">c=2&#124;M_0&#124;\leq2\nu(G)</code>。边数与工作量界直接来自构造。∎

对于任意局部邻居选择，截断值 <code class="math">c</code> 已不能再降低。取偶数 <code class="math">c\geq2</code>，令 <code class="math">C</code> 与独立集 <code class="math">I</code> 各有 <code class="math">c</code> 个顶点，在二者之间取完全二分图，并在 <code class="math">C</code> 内加入一个完美匹配。该匹配可以作为初始极大匹配。若每个覆盖顶点都保留 <code class="math">I</code> 中相同的 <code class="math">c-1</code> 个顶点，则保留图仅有 <code class="math">2c-1</code> 个非孤立顶点，匹配数为 <code class="math">c-1</code>；而原图匹配数为 <code class="math">c</code>。

**推论 3.1a（按连通分量细化）。** 令 <code class="math">G_1,\ldots,G_a</code> 为清理后图的非空连通分量，令 <code class="math">m_i</code> 和 <code class="math">\nu_i</code> 分别为其边数和匹配数。通过一次 <code class="math">O(n+m)</code> 扫描，分别构造各分量的边核，其保留规模为

<!-- math-display: ad0a5dc30399fd01705c24bb
h_i\\leq\\min\\left\\{m_i,\\binom{c_i}{2}+c_i^2\\right\\}
       =O(\\min\\{m_i,\\nu_i^2\\}).
-->
![公式](../../assets/math/ad0a5dc30399fd01705c24bb.svg)

对每个保留分量应用定理 1.1，可得细化后的运行时间

<!-- math-display: c11b4819087eae338a95a968
O(n+m)+\\sum_{i=1}^{a}(h_i+1)
 \\exp\\!\\left[O\\!\\left(
   (\\log(h_i+2))^{4/5}(\\log\\log(h_i+4))^{1/5}
 \\right)\\right].
-->
![公式](../../assets/math/c11b4819087eae338a95a968.svg)

**证明。** 连通分量与所有贪心匹配总共在线性时间内计算。对每个分量应用引理 3.1，并在每个保留图上调用同一个统一程序。最大匹配数对连通分量可加，因此解码后各匹配的并集仍是最大匹配。每个非空分量至少保留一条边，故分量调用的额外开销和最终输出均已计入。所有调用使用相同常数。∎

后续所有工作器和验证器均使用同一个清理后图或同一个边核。原始标识仅作为被动标签保留，直至最终解只进行一次映射。

### 3.2 对任意匹配均可解码的精确次三次约简

对待求解的图，记其边数和非孤立顶点数为 <code class="math">e</code>、<code class="math">v</code>。将度数为 <code class="math">d_u</code> 的顶点 <code class="math">u</code> 替换成一条含 <code class="math">2d_u-1</code> 个顶点的路径。路径上的奇数位置作为关联端口。每条原图边连接其指定的两个端口。令 <code class="math">J</code> 为所得次三次图，并定义

<!-- math-display: c5fa7462e95b864791034b6c
A=\\sum_u(d_u-1)=2e-v,\\qquad N=4e-v.
 \\tag{3}
-->
![公式](../../assets/math/c5fa7462e95b864791034b6c.svg)

**引理 3.2。** 有 <code class="math">\nu(J)=A+\nu(G)</code>。对 <code class="math">J</code> 的任意匹配 <code class="math">Q</code>，均可在线性时间内解码为 <code class="math">G</code> 的一个匹配，其基数至少为 <code class="math">&#124;Q&#124;-A</code>。

**证明。** 将原图匹配提升时，保留被占用的端口，并分别匹配其两侧偶数长度的路径段。未匹配的组件留下第一个端口空闲。每个组件贡献 <code class="math">d_u-1</code> 条内部边，由此得到一个方向的不等式。

反过来，令 <code class="math">F</code> 为 <code class="math">Q</code> 中连接不同组件的边集，并将其视为原顶点上的边集；令 <code class="math">t_u=\deg_F(u)</code>。若 <code class="math">t_u=0</code>，该组件至多贡献 <code class="math">d_u-1</code> 条内部匹配边。若 <code class="math">t_u\geq1</code>，删除已被占用的奇数端口后，剩余路径段的总匹配容量为 <code class="math">d_u-t_u</code>。因此

<!-- math-display: 637f1a9af7213b23487b15b0
|Q|\\leq |F|+\\sum_u(d_u-\\max\\{1,t_u\\})
       =A+|V(F)|-|F|.
 \\tag{4}
-->
![公式](../../assets/math/637f1a9af7213b23487b15b0.svg)

这里 <code class="math">V(F)</code> 不包含孤立顶点。<code class="math">F</code> 的每个非空树分量对 <code class="math">&#124;V(F)&#124;-&#124;F&#124;</code> 的贡献为 1；其他每个分量的贡献至多为 0。从每个树分量选一条边即可组成原图匹配，其基数至少为 <code class="math">&#124;Q&#124;-A</code>。遍历各分量即可完成解码。将该解码应用于最大 <code class="math">Q</code>，结合两个方向的不等式即得结论。∎

选取 <code class="math">S=2^s</code> 使 <code class="math">N\leq S&lt;2N</code>；有界规模情形直接处理。将贪心匹配提升后作为阶段驱动器的初始匹配。由式 (3)，其剩余最优差距至多为 <code class="math">\lfloor v/2\rfloor-&#124;M_0&#124;</code>。

## 4. 被表示的匹配与极小紧集收缩

### 4.1 箱、行与账本

一个*箱* <code class="math">d</code> 是奇数阶、因子临界的物理块，大小为 <code class="math">2w_d-1</code>，其中 <code class="math">w_d\geq1</code>。一个*行*是箱外的单个物理顶点，且最多有三个与箱相接的物理关联。令 <code class="math">\mathcal A</code> 表示行集合，<code class="math">\mathcal D</code> 表示箱集合。单射 <code class="math">\mu:\mathcal A\to\mathcal D</code> 将每一行沿一条真实关联指派给一个箱。其余顶点显式地匹配成对，或作为未匹配阻塞点。

每个被指派的箱暴露其被指派关联的端点，与该行匹配，并在箱内匹配其余顶点。空闲箱暴露一个未匹配顶点。因此表示出的匹配基数为

<!-- math-display: 7bb9d6321c591d083ee02104
|Q|=\\sum_{d\\in\\mathcal D}(w_d-1)+|\\mathcal A|
                      +\\tfrac12|V_{\\mathrm{paired}}|.
 \\tag{5}
-->
![公式](../../assets/math/7bb9d6321c591d083ee02104.svg)

每一个有效单射都表示同一基数的匹配。

对于箱集 <code class="math">X</code>，定义

<!-- math-display: 4a345dadd789377866cb971b
\\mathcal A(X)=\\{a:N(a)\\subseteq X\\},\\qquad
 \\eta(X)=|X|-|\\mathcal A(X)|.
 \\tag{6}
-->
![公式](../../assets/math/4a345dadd789377866cb971b.svg)

底数为 <code class="math">c</code> 的账本包含非负关联分配 <code class="math">x</code>、剩余量 <code class="math">y_d=L-\sum_a x_{ad}</code>，以及非负债务 <code class="math">p_d</code>，并满足

<!-- math-display: 0449b8d83fb624920f3b982c
\\sum_d x_{ad}=L,\\qquad y_d+p_d\\geq c w_d.
 \\tag{7}
-->
![公式](../../assets/math/0449b8d83fb624920f3b982c.svg)

整数层的值均为整数。在已稳定的底层，<code class="math">c=1</code> 且 <code class="math">p=0</code>。因此，对任意非空 <code class="math">X</code>，

<!-- math-display: 3edb9d1a0e891291665fdcf1
L\\eta(X)\\geq y(X)\\geq w(X)>0.
 \\tag{8}
-->
![公式](../../assets/math/3edb9d1a0e891291665fdcf1.svg)

特别地，<code class="math">\eta(X)\geq1</code>；若 <code class="math">X</code> 是*紧集*，即 <code class="math">\eta(X)=1</code>，则 <code class="math">w(X)\leq L</code>。单个箱的权重也不超过 <code class="math">L</code>。

顶层的正坐标为 <code class="math">x^0</code> 和 <code class="math">\sigma_d=y_d^0+p_d^0-Cw_d</code>。其对数目标值为这些坐标的对数之和。下面的整数载体归一化不会改变顶层坐标。

### 4.2 构造性收缩

**引理 4.1。** 设一条物理边连接不同的箱 <code class="math">i,j</code>。若 <code class="math">X</code> 是包含 <code class="math">i,j</code> 的包含关系极小紧集，则由其箱及其受困行 <code class="math">\mathcal A(X)</code> 组成的并集是因子临界的，权重为 <code class="math">w(X)</code>，且收缩后表示的匹配基数不变。

**证明。** 其中有 <code class="math">&#124;X&#124;-1</code> 个受困行，所以物理块大小为 <code class="math">2w(X)-1</code>。对任意非空受困行集 <code class="math">R</code>，将严格盈余条件用于 <code class="math">N(R)</code>，得 <code class="math">&#124;N(R)&#124;\geq &#124;R&#124;+1</code>。因此删去任意一个子箱后仍满足 Hall 条件。这保证可以找到一个受困行单射，避开含指定暴露物理顶点的子箱。暴露该子箱中的指定顶点；在其他每个子箱中，暴露其被指派行所接触的顶点。

若要暴露一个受困行 <code class="math">a</code>，则移除它，并将 <code class="math">i,j</code> 留给连接边。若其余受困行无法单射到 <code class="math">X\setminus\{i,j\}</code>，则存在行集 <code class="math">R\subseteq\mathcal A(X)\setminus\{a\}</code>，满足 <code class="math">&#124;N(R)\setminus\{i,j\}&#124;&lt;&#124;R&#124;</code>。严格盈余条件迫使 <code class="math">i,j\in N(R)</code>，且 <code class="math">&#124;N(R)&#124;=&#124;R&#124;+1</code>，恰好这些行受困。这就得到一个包含 <code class="math">i,j</code> 的真紧子集，与极小性矛盾。将预留的接点通过连接边匹配，将其余行匹配进各自子箱，并只留下 <code class="math">a</code> 暴露。

旧单射将 <code class="math">&#124;X&#124;-1</code> 个箱用于受困行，因此至多有一个外部行被指派进 <code class="math">X</code>。若存在，则将该关联保留为新箱的外部指派。式 (5) 保持不变。∎

每次收缩保存一个不可变记录：子箱记录、受困行关联、连接边，以及创建时的受困行指派。要物化指定暴露，只需修复该存储指派；暴露子箱时至多做一次二分图增广，暴露被吞并的行时至多做两次。每行至多有三个关联。记录树的总分支数与物理体积线性相关，因此解散成本为该体积乘以对数级索引因子。之后的全局指派选择不会修改这些创建时记录。

## 5. 实际整数账本的持久化载体

### 5.1 稀疏批次归一化

载体是行／箱二分关联图中的森林。它包含所有正分配；其他载体边的分配可以为零。所有非载体关联的分配均为零。实际整数账本以这种形式存储。

**引理 5.1（载体归一化）。** 对涉及 <code class="math">q</code> 个关联的合法旧值到新值批次（包括关联的创建、删除或端点变更），可通过 <code class="math">O(q)</code> 次带符号路径操作、删边与连边来更新并归一化。所得账本具有批次指定的行列边际，且所有值均为非负整数。

**证明。** 从指定版本读取所需旧值，并合并重复的最终写入。先应用整个批次，再执行枢轴操作。切断已删除或端点改变的载体边，更新保留边值，并将至多 <code class="math">O(q)</code> 个载体外的正值放入待处理字典。

逐一处理待处理关联 <code class="math">e=(a,d)</code>。若端点不连通，则直接连边。否则，沿载体中从行 <code class="math">a</code> 到箱 <code class="math">d</code> 的唯一简单路径遍历，并永久按“行到箱”方向给每个关联定向。令 <code class="math">\theta</code> 为路径上顺着遍历方向的边的最小值；该类别非空。将 <code class="math">\theta</code> 加到 <code class="math">e</code> 上；对顺向路径边减去该值，对逆向路径边加上该值。这是一个运输环流，因而保持全部边际不变。切断一条已降为零的顺向最小边，并连入 <code class="math">e</code>。其余所有零边均保留。最小值为零也是允许的。

每个待处理关联至多使用一次枢轴。较早的枢轴只作用于载体边和它自己的待处理边，因此不会改变尚未处理的待处理值。每一步均保持非负性、整数性和无环性。∎

某条边可能在没有显式通知的情况下变成零。它会一直保留，直到某个已计费的拓扑操作将其删除。账本字段 <code class="math">y,p,w</code> 不受枢轴影响，因此式 (7)、(8) 精确保持。

### 5.2 平衡的所选树与规范点指派

将每个载体连通分量以标识符最小的箱为根。每个行至少有两个正关联：若只有一个正关联，其值就会等于 <code class="math">L</code>，超过其箱容量 <code class="math">L-w_d&lt;L</code>。因此载体度数为 2 或 3。对度数为 2 的行保留其两个关联；对度数为 3 的行保留指向其两个子节点的关联。抑制这些行后得到所选森林，每行对应一条带行标签的边。

**引理 5.2。** 每个所选连通分量 <code class="math">T</code> 满足 <code class="math">w(T)&lt;2L</code>。包括内部行在内，其物理体积小于 <code class="math">4L</code>。

**证明。** 设所选分量的箱集为 <code class="math">D_T</code>，其中有 <code class="math">&#124;D_T&#124;-1</code> 个内部行。至多一个内部行（若存在则为顶部三度行）会向分量外发送分配。令其输出量为 <code class="math">o</code>。由于至少有一个保留的子关联为正，故 <code class="math">o&lt;L</code>。令 <code class="math">i\geq0</code> 为外部行流入的分配量。于是

<!-- math-display: c76570cc62e91c27bf17d940
\\mathrm{load}(D_T)=L(|D_T|-1)-o+i
                         \\leq L|D_T|-w(D_T).
-->
![公式](../../assets/math/c76570cc62e91c27bf17d940.svg)

因此 <code class="math">w(D_T)\leq L+o-i&lt;2L</code>。物理体积为 <code class="math">2w(D_T)-1</code)。∎

将度为 2 的载体行指派给其子箱；将度为 3 的行指派给关联标识符较小的子箱。每个箱至多只有一个载体父行，因此该指派是单射。要在箱处反向查询时，只需检查其父行以及该行所选的子节点。这类查询只需常数次根化森林操作和至多三个关联，从而隐式表示完整的当前单射。

### 5.3 持久化与事务顺序

使用 [AHLT] 的最坏情形动态树实现。路径簇存储两个方向类别、两个最小值及两个惰性标记；路径反转时交换两个类别。内部的度数约简连接点的关联数为零，不参与分配极值或加法操作。

用一个从地址到字的持久化平衡字典模拟其内存。每次写入都对字典路径进行复制，一个版本由一个字典根表示。额外的对数因子仍使操作时间保持多对数级。冻结版本上的只读查询可在临时持久分支中执行，随后丢弃该分支。所有地址和机器字操作均计费。

对每一行版本化其完整的至多三个关联列表，包括非载体上的零值选项、物理接触点及生命周期端点。历史查询读取的任何邻接字典也必须版本化。由此旧版本成为完整的关联状态。

结构事务按以下顺序执行。

1. 冻结事件发生前的底层指派版本，并为事件建立稀疏的正向／逆向覆盖层。
2. 收缩前保存受困指派。所有被指派行的删除均使用同一版本及覆盖层。
3. 在阈值测试、锁定、容量读取及最终恢复期间，冻结每个投影的父状态。
4. 对子状态应用完整的分配批次，并在归一化前释放所有已删除行的负载。保留性的删除则分别应用于更粗层账本。
5. 整个事务稳定后，发布新的底层规范单射。

未变的物理匹配边通过因子临界记录保持隐式。只有被抹除的物理体积和阶段边界输出会被物化。式 (5) 说明，可以用另一个稳定的有效单射替换原单射。

## 6. 确定性局部结构操作

### 6.1 输出敏感的极小紧集发现

对于查询箱 <code class="math">i,j</code>，构造一个隐式的有向拆点图。每个箱拆为 <code class="math">d_{\rm in}\to d_{\rm out}</code>。从查询源点到 <code class="math">i_{\rm in},j_{\rm in}</code> 添加重边；从有占用的 <code class="math">d_{\rm out}</code> 到其被指派行的每个邻居输入点添加重边；从空闲的 <code class="math">d_{\rm out}</code> 到汇点添加重边。出度为常数。通过逆向指派查询以及被指派行的三个关联生成这些边。

对包含 <code class="math">i,j</code> 的紧集 <code class="math">X</code>，恰有 <code class="math">&#124;X&#124;-1</code> 个箱被受困行占用。纳入 <code class="math">X</code> 的所有输入点、查询源点，以及这些被占用箱的输出点。此时恰有一条出割边，诱导边数为 <code class="math">O(&#124;X&#124;+1)</code>。

按倍增预算调用 [CHHIP]，预算上限取 <code class="math">L</code> 的常数倍。若延迟生成的重边链超过最大返回体积预算的两倍，则其长度足以排除包含汇点的输出。先将返回集限制到原诱导图中从查询源点可达的顶点。由于不存在重边能够跨过其单边边界，输入箱便构成包含 <code class="math">i,j</code> 的紧集见证。

只使用该见证中的受困行对其进行细化：在其拆点网络中做一次单位增广，随后求残余可达集，即可得到最小的源侧最小割。包含该点对的紧集对交封闭，因为 <code class="math">\eta</code> 是次模函数，且非空交集满足严格盈余。因此这就是全局包含关系极小的紧集见证。倍增使得：返回紧集时总工作量为 <code class="math">O((&#124;X&#124;+1)\mathrm{polylog}S)</code>；对经认证可释放的点对，工作量为 <code class="math">O(L\mathrm{polylog}S)</code>。

### 6.2 在两棵小型所选树内联合暴露

若经认证可释放的一对箱分别位于不同的所选树，则枚举这两棵树，并以该箱对为根。否则枚举其共同的所选树 <code class="math">T</code>，其物理体积为 <code class="math">O(L)</code>。任何同时包含查询箱的紧集都包含于 <code class="math">T</code> 中：其 <code class="math">&#124;X&#124;-1</code> 条受困所选边已经连通了其中的箱。

在并行轮次中剪除行边：若某行有第三个邻居位于当前分量之外，则剪除此行边。记录剪除轮次和一个外部见证。通过队列处理并扫描较小的子块，工作量为 <code class="math">O(&#124;T&#124;\mathrm{polylog}S)</code>。最终各分量是极大的紧块：在终止分量中，每条所选行边都被受困；而第一条被剪除的内部边阻止任何紧集跨越该划分。

加入一条层级为无穷大的人工边 <code class="math">ij</code>，在其环上删除一条层级最小的有限层行边，并将该行视为一个空位。用一对边（原端点及其记录的外部见证）替换空位的原边。若此操作形成环，则在该环上删除层级最低的边并继续。空位层级严格下降。事实上，在阈值 <code class="math">\ell</code> 处，每条当前层级至少为 <code class="math">\ell</code> 的边都位于原始的第 <code class="math">\ell</code> 层分量内，而空位见证在该分量之外。因此任何连接路径都必须经过一条更低层级的边。最后一步连接到第二棵原始树；删去人工边后，<code class="math">i,j</code> 被分离。

除最后一次连接外，所有交换都留在 <code class="math">T</code> 内。只有最后连接时才枚举第二棵树。有限层级数至多为 <code class="math">O(L)</code>，所有触及的物理总体积为 <code class="math">O(L)</code>。将所得两棵树定向以释放该点对，并记录显式指派覆盖层。该覆盖层立即用于增益操作；下一个稳定账本将提供其自身平衡的载体森林。

因此，对收缩 <code class="math">X_1,\ldots,X_b</code> 及 <code class="math">g</code> 次增益，局部点对工作量为

<!-- math-display: 496158d1f0d97b8de44680e5
O\\!\\left(\\left[\\sum_{a=1}^{b}(|X_a|-1)+Lg\\right]
                     \\mathrm{polylog}S\\right).
 \\tag{9}
-->
![公式](../../assets/math/496158d1f0d97b8de44680e5.svg)

## 7. 精确最优差距阶段与二进制账本记账

### 7.1 阶段规格

阶段开始时，表示的匹配大小为 <code class="math">q_0</code>，目标增益为 <code class="math">K_{\mathrm g}\geq1</code>，并令

<!-- math-display: 55cba0ac78222572cc820cf7
C=s+1,\\qquad
 L=\\left\\lceil\\frac{10000CN}{K_{\\mathrm g}}\\right\\rceil,\\qquad
 g_*=\\max\\left\\{1,\\left\\lfloor\\frac{K_{\\mathrm g}}{100C}\\right\\rfloor\\right\\}.
 \\tag{10}
-->
![公式](../../assets/math/55cba0ac78222572cc820cf7.svg)

若 <code class="math">K_{\mathrm g}&gt;\lfloor N/2\rfloor-q_0</code>，则立即返回 LOW，并给出其基数理由。否则，将未匹配顶点初始化为单点箱，将已匹配点对作为剩余配对部分，且不设置行或阻塞点。初始债务为零。使用层级 <code class="math">j=0,\ldots,s</code>，其底数为 <code class="math">c_j=C-j</code>；底层底数为 1。

在稳定状态下，执行第一个适用的操作。

1. **箱间接触。** 对于连接两个箱的边，若存在极小紧集见证，则收缩；否则释放该点对、暴露连接端点、沿该边匹配，并将两个箱还原为剩余配对部分。这使匹配基数增加 1。
2. **扩展。** 若某箱与剩余配对部分接触，且缓存的顶层松弛至少为 <code class="math">4C</code>，则将接触顶点设为一行，其配偶设为单点箱，并将该行指派给新箱。
3. **定价。** 请求顶层对数坐标上的一次定价环流步骤。精确应用返回步骤并处理通知。有效的 LOW 结果会结束该阶段。

达到第 <code class="math">g_*</code> 次增益时立即结束阶段。其他结构操作之后执行下面的重置事务。若经检查的前置条件或工作上限违反，则暂停该工作器；这与 LOW 是不同的结果。

### 7.2 账本代数与被抹除体积的收费

紧缩将 <code class="math">X</code> 替换为权重 <code class="math">w(X)</code>、剩余量 <code class="math">y(X)</code>、债务 <code class="math">p(X)</code> 的箱。事实上，删除 <code class="math">&#124;X&#124;-1</code> 个受困行会移除 <code class="math">L(&#124;X&#124;-1)</code> 的负载，使新剩余量为 <code class="math">L&#124;X&#124;-\mathrm{load}(X)=y(X)</code>。所有层级不等式可直接相加。

整数扩展将 <code class="math">L</code> 分配给新指派关联，并在子箱上增加债务 <code class="math">c_j</code>。在顶层，将 <code class="math">C+1</code> 放在触发父关联上，将 <code class="math">L-C-1</code> 放在子关联上；其余坐标使用来源中的正性服务。资格条件保证父箱具有足够的实际松弛；新子箱的松弛为 1。阶段内受保护的正性服务所加的总债务小于 <code class="math">S^{-800}</code>。

要抹除箱集 <code class="math">R</code>，在冻结的事件指派下，删除所有指派进 <code class="math">R</code> 的行。将每个幸存行从 <code class="math">R</code> 转移的分配重定向到其被指派的外部箱，并在该箱上增加同额债务。若 <code class="math">R</code> 中有 <code class="math">f</code> 个空闲箱，则删除 <code class="math">&#124;R&#124;-f</code> 行。令 <code class="math">I</code> 为其旧的外部关联分配总量，<code class="math">O</code> 为重定向后的分配总量，则

<!-- math-display: 5e4dd1fb7f8fac9c421e770e
y(R)=Lf+I-O,\\qquad
 I\\leq L(\\eta(R)-f).
-->
![公式](../../assets/math/5e4dd1fb7f8fac9c421e770e.svg)

扣除删除的债务并使用式 (7)，得到

<!-- math-display: 0fd0bcb25863cc9879c658d9
\\Delta p_{\\rm total}=O-p(R)\\leq L\\eta(R)-c_jw(R).
 \\tag{11}
-->
![公式](../../assets/math/0fd0bcb25863cc9879c658d9.svg)

对于一次增益，两个被抹除的箱都是空闲的，因此债务增加至多 <code class="math">2L</code>。投影到底数 <code class="math">c</code> 失败时，第 8 节证明会给出完整残余剥离集 <code class="math">R</code>，满足 <code class="math">L\eta(R)&lt;cw(R)</code>。在每个被保留的更粗层账本中，<code class="math">c_j\geq c+1</code>；因此由式 (11)，债务至少减少 <code class="math">w(R)</code>。该剥离操作产生

<!-- math-display: 756e12320baca1992e6ce86f
f\\leq\\eta(R)<\\frac{Cw(R)}{L}
 \\tag{12}
-->
![公式](../../assets/math/756e12320baca1992e6ce86f.svg)

个新阻塞点。

在终止增益之前，顶层债务支付所有被剥离的权重。因此

<!-- math-display: 24140cd5c67988686b560af7
\\sum_{\\rm peels}w(R)\\leq2L(g_*-1)+S^{-800},
 \\qquad
 \\#\\mathrm{blockers}\\leq2C(g_*-1).
 \\tag{13}
-->
![公式](../../assets/math/24140cd5c67988686b560af7.svg)

第二个界来自式 (12) 的严格不等式、整数性，以及 <code class="math">CS^{-800}/L&lt;1</code>。一次剥离至多物化 <code class="math">2w(R)</code> 个物理顶点；一次增益抹除的箱总权重至多为 <code class="math">2L</code>。又有 <code class="math">Lg_*\leq10001CN</code>。计入初始顶点、返回体积、扩展和收缩分支数后，结构总体积为 <code class="math">O(CN)</code>。在每次收缩中保留最大的子块，可使一个物理顶点在箱系统中停留期间移动至多 <code class="math">O(\log S)</code> 次。其原始度数至多为三。因此，除数值投影工作外，所有物理关联工作及所有层级的载体更新总成本为 <code class="math">S\mathrm{poly}(s)</code>。

### 7.3 比较匹配排除错误的 LOW

**引理 7.1。** 若 <code class="math">\nu(J)\geq q_0+K_{\mathrm g}</code>，则阶段不可能在第 <code class="math">g_*</code> 次增益之前返回有效 LOW。

**证明。** 在进行了 <code class="math">i&lt;g_*</code> 次增益后，将当前表示匹配与一个大小至少为 <code class="math">q_0+K_{\mathrm g}</code> 的比较匹配作比较。两者的对称差中至少包含 <code class="math">K_{\mathrm g}-i</code> 条顶点不相交的增广路。丢弃所有与阻塞点相交的路径。由式 (13)，至少还剩下

<!-- math-display: 49f76f82798e08ab9ec480c3
K_{\\mathrm g}-i-2C(g_*-1)\\geq K_{\\mathrm g}/2
 \\tag{14}
-->
![公式](../../assets/math/49f76f82798e08ab9ec480c3.svg)

条路径。

在没有箱间边的稳定状态下，将每条剩余路径从未匹配端点定向。路径从空闲箱的暴露基点开始。进入一行后，其下一条匹配边是该行的指派关联，进入下一个箱的唯一暴露基点。该箱的交叉匹配边已经使用，所以简单交替路径必须沿一条非匹配边离开。在到达剩余配对部分之前，每次进入箱都必须经由其基点。

这种前缀不能在已指派箱处结束，也不能通过已指派行进入另一个空闲箱。它必须抵达一个与剩余配对部分接触的箱。不同物理路径的箱基点和行互不相交。将每个前缀中的行重新指派给其前一个箱。由此得到的概念单射释放了 <code class="math">r_0\geq K_{\mathrm g}/2</code> 个不同的接触箱。

将概念单射乘以 <code class="math">L</code>。其顶层列容量是 <code class="math">L+p_d^0-Cw_d</code>，由顶层可行性可知非负。截去超载列；被移除的总质量至多为 <code class="math">CN</code>。现有可行顶层分配保证可以在这些容量下完成。源汇残余增广不会减少列汇流量，因此起初为空的接触列最多获得被移除的 <code class="math">CN</code> 质量。接触箱集 <code class="math">Z</code> 上的替代顶层松弛满足

<!-- math-display: 4570f1aff95a8d15d66d0dce
\\sigma'(Z)\\geq Lr_0-2CN.
 \\tag{15}
-->
![公式](../../assets/math/4570f1aff95a8d15d66d0dce.svg)

这只是一个存在性论证。算法不会为了获得该比较方向而实际求流。

若没有可执行的扩展，来源缓存保证 <code class="math">Z</code> 上有 <code class="math">\sigma_d\leq5C</code>。分配弧和松弛弧都进入箱，因此替代坐标与当前坐标之差构成一个环流。每个负对数导数项的绝对值至多为 1，因为替代坐标非负。这样的项至多有 <code class="math">4N</code> 个。正贡献至少为

<!-- math-display: 0fc89ef1514d635bbf9eab94
\\frac{Lr_0-2CN}{5C}-N>12N.
 \\tag{16}
-->
![公式](../../assets/math/0fc89ef1514d635bbf9eab94.svg)

其带符号导数大于各单项导数绝对值之和的一半。第 8.2 节的定价步接口在此情形下排除 LOW。由于目标值范围有界，且每次接受步骤都有正进展，也不可能无限定价。∎

因此，在进行了 <code class="math">i</code> 次增益后返回 LOW，恰好意味着 <code class="math">\nu(J)&lt;q_0+K_{\mathrm g}</code>，而当前匹配大小为 <code class="math">q_0+i</code>。

### 7.4 二进制重置与可加的投影体积

给第 <code class="math">j</code> 层设置周期 <code class="math">b_j=2^{s-j}</code>。扩展与收缩各增加一个计数；已修复的增益增加 <code class="math">L</code>；剥离不增加计数。当计数正向增加时，重置满足周期商首次变化的最小 <code class="math">j\geq1</code>。第 1 层重置使用全扩展的精确交换恢复。在 <code class="math">j&gt;1</code> 时，从刚刚变为当前且已归一化的父状态进行投影；整个调用期间冻结父状态，并与更深层新建的层共享稳定子状态的持久化根。

总计数为 <code class="math">\tau=O(sS)</code>。若选中 <code class="math">j&gt;1</code>，则没有跨过 <code class="math">b_{j-1}</code> 的边界。由于扩展每计数至多增加 <code class="math">C</code> 债务，增益至多增加计数变化量的两倍，故父状态债务满足

<!-- math-display: dee84d878e8cf1d6ea10de4a
P_{\\mathrm d}\\leq(C+2)b_{j-1}.
 \\tag{17}
-->
![公式](../../assets/math/dee84d878e8cf1d6ea10de4a.svg)

这也适用于较大的计数跳跃：若跨过更粗层边界，就会选择该更粗层重置。第 <code class="math">j</code> 层至多被选中 <code class="math">\tau/b_j</code> 次，且 <code class="math">b_{j-1}=2b_j</code>。因此

<!-- math-display: dc1fbe277a3564b4eaa55ae2
\\sum_{\\text{higher resets}}(P_{\\mathrm d}+2)
 \\leq\\sum_{j=2}^{s}\\frac{\\tau}{b_j}
                         ((C+2)b_{j-1}+2)
 =O(s^3S).
 \\tag{18}
-->
![公式](../../assets/math/dc1fbe277a3564b4eaa55ae2.svg)

第 1 层重置次数为 <code class="math">O(s)</code>。结构批次应用到 <code class="math">s</code> 个账本会带来可加因子；数值精度将在后文独立选取。

### 7.5 最优差距驱动器

维护整数 <code class="math">B\geq\nu(J)-&#124;Q&#124;</code>。当 <code class="math">B&gt;0</code> 时，置 <code class="math">K_{\mathrm g}=\lceil B/2\rceil</code> 并运行一个阶段。

~~~text
SUCCESS，产生 g 次增益：     B <- B - g
LOW，在 i 次增益后返回：      B <- K_g - 1 - i
SUSPENDED：                  暂停该工作器
~~~

根据引理 7.1 和整数性，这两种数值更新都保持上界条件。当 <code class="math">B</code> 大于 <code class="math">C</code> 的固定倍数时，SUCCESS 会使其按 <code class="math">\Omega(1/C)</code> 的比例下降，而 LOW 会使其大致减半。在该阈值以下，每次 SUCCESS 至少增加 1。阶段数为 <code class="math">O(C(1+\log N))=O(s^2)</code>。当 <code class="math">B=0</code> 时，物化最大 gadget 匹配并应用引理 3.2。

## 8. 从原始环流恢复精确投影

### 8.1 原始数值接口

对一个有向图，设正整数边长为 <code class="math">\ell</code>，整数梯度为 <code class="math">\gamma</code>，定义

<!-- math-display: 1ccfa9e8c19383e98dfcc157
\\rho^*=\\sup_{\\substack{Bc=0\\\\c\\ne0}}
           \\frac{|\\gamma^{\\mathsf T}c|}{\\sum_e\\ell_e|c_e|}.
 \\tag{19}
-->
![公式](../../assets/math/1ccfa9e8c19383e98dfcc157.svg)

其中 <code class="math">B</code> 为关联矩阵；若不存在非零环流，则取 <code class="math">\rho^*=0</code>。后端答案由带符号的单位真实边片段以及已暴露出现项森林中的带符号路径组成。它们的总和投影为环流 <code class="math">c</code>。证书 <code class="math">V</code> 支配**抵消前所有映射后遍历**的长度总和；所需答案保证为

<!-- math-display: 2c38189940759e4c12e8da1a
\\frac{|\\gamma^{\\mathsf T}c|}{V}\\geq\\frac{\\rho^*}{E}.
 \\tag{20}
-->
![公式](../../assets/math/2c38189940759e4c12e8da1a.svg)

零分数情形可以返回有证书的零备选项。完整上限 <code class="math">E</code> 还约束每条实际记录的初始化、摊还编辑与查询、暴露日志、出现项重数及答案片段。该接口涵盖不连通图、真正的自环、原生标识、稳定路径映像、非活跃出现项以及完整退役转移。数值实例的实际名义规模至多为 <code class="math">S^2</code>，根价格为 <code class="math">O(s)</code> 位，并有固定多项式的主历史。定理 11.6 构造该接口，并给出定理 1.1 所用的界。

### 8.2 直接采用主规模参数的定价客户端

令

<!-- math-display: 487a81053292c35577053c18
D=\\mathrm{pow2ceil}(\\max\\{16,4E\\}),\\qquad
 q=\\frac1{16D},\\qquad
 \\delta=S^{-1000},\\quad \\varepsilon=S^{-3000},
 \\quad H_{\\rm price}=S^{100}.
 \\tag{21}
-->
![公式](../../assets/math/487a81053292c35577053c18.svg)

对数客户端的坐标梯度为 <code class="math">g</code>，绝对导数权重为 <code class="math">\lambda</code>；绝对权重按发生抵消前的每个对数项计数。在锚点处取整数价格 <code class="math">\ell_e=\lceil H_{\rm price}\lambda_e\rceil</code> 和 <code class="math">\gamma_e=\mathrm{round}(H_{\rm price}g_e)</code>。来源论证及幅值界给出 <code class="math">\ell_e\geq S^{80}</code>，且价格小于 <code class="math">S^{1200}</code>。度数为三的私有端口路径，会使逻辑环流长度至多增加 <code class="math">2M\sum_e|d_e|</code>，其中 <code class="math">M\leq S^2</code>。因此任一逻辑方向

<!-- math-display: 2e3c7e8b67cdaa680618aa28
 g^{\\mathsf T}d>\\tfrac12\\sum_e\\lambda_e|d_e|
 \\tag{22}
-->
![公式](../../assets/math/2e3c7e8b67cdaa680618aa28.svg)

都有整数价格提升比率大于 <code class="math">1/4</code>。结合式 (20)、<code class="math">E\leq D/4</code> 以及 <code class="math">1/(4D)&gt;2q</code>，它通过接受检验 <code class="math">|\gamma^{\mathsf T}c|\geq2qV</code>。这证明了前文使用的“不返回 LOW”蕴含。

选取一个共同的二进制步长系数 <code class="math">\mu</code>，使得

<!-- math-display: 7b85ffd0b2865a29039af633
\\frac q{400}\\leq\\frac{\\mu V}{H_{\\rm price}}\\leq\\frac q{200}.
 \\tag{23}
-->
![公式](../../assets/math/7b85ffd0b2865a29039af633.svg)

原始遍历界控制路径列表每个部分应用阶段，因此全过程保持正性。坐标漂移阈值是二进制数值，满足

<!-- math-display: dd6ea5d264388875145ad3c5
 \\frac{qH_{\\rm price}}{800D\\ell_e}
 \\leq\\theta_e\\leq
 \\frac{qH_{\\rm price}}{400D\\ell_e}.
 \\tag{24}
-->
![公式](../../assets/math/dd6ea5d264388875145ad3c5.svg)

此时，来源中的描述符／Taylor 论证给出精确的 <code class="math">\varepsilon</code>-网格环流步，总相对绝对移动量至多为 <code class="math">q/100</code>，目标值增量为 <code class="math">\Omega(q^2)</code>，稳定寄存器误差至多为 <code class="math">q/100</code>。一次报告至少消耗 <code class="math">q/(800D)</code> 的原始定价移动，而一次步进最多产生 <code class="math">q/200</code>；因此每步真正的报告数为 <code class="math">O(D)</code>。共同网格兼容性及这些结论需要检验 <code class="math">q\geq S^{-1}</code>、<code class="math">D\leq S^2</code>；下文得到的次多项式上限最终保证二者成立。

令 <code class="math">J_{\rm step}</code> 为已接受步数，令 <code class="math">Q_{\rm own}</code> 为所属客户端发出的请求数，包括 LOW 与未成功请求。内部刷新另行计数。完整初始化、点访问、复制扫描与刷新工作满足

<!-- math-display: 8fbc544dd36070956765e435
T_{\\rm priced}
 \\leq O(MD+Q_{\\rm own}D+D^2J_{\\rm step})
                                      \\mathrm{poly}(s).
 \\tag{25}
-->
![公式](../../assets/math/8fbc544dd36070956765e435.svg)

所有暴露副本（包括非活跃副本）均保留在带符号与绝对移动列表中。退役前，两个计数器都必须转移。获取查询答案并消耗或丢弃它属于一个事务；由表示变化触发的刷新排在其后。可变数值树只有一段摊还历史。持久化整数载体则是另一种最坏情形结构。

### 8.3 将总违反量小于 1 的守恒近可行解精确舍入

**引理 8.1（守恒近可行舍入）。** 设 <code class="math">f</code> 在有向多重图中精确满足整数顶点需求，且边的整数下界和上界为 <code class="math">l\leq u</code>。若

<!-- math-display: 02b7779da1cd629a1df3c713
\\mathrm{viol}(f)=
 \\sum_e((l_e-f_e)_++(f_e-u_e)_+)<1,
 \\tag{26}
-->
![公式](../../assets/math/02b7779da1cd629a1df3c713.svg)

则存在确定性算法，在 <code class="math">O(m\log(m+2))</code> 次标量／动态树操作内找到具有相同需求的整数可行向量；若输入还给出孤立顶点，则额外需要 <code class="math">O(n+m)</code> 的输入／需求簿记。对 <code class="math">b</code> 位二进制有理数输入，算术只需 <code class="math">O(b+\log(m+2))</code> 位。

**证明。** 将坐标 <code class="math">e</code> 限制在区间 <code class="math">[\lfloor f_e\rfloor,\lceil f_e\rceil]</code>。在该盒内，原始违反量可写成 <code class="math">a^{\mathsf T}x+\kappa</code>，其中常数 <code class="math">\kappa</code> 为整数，且按原坐标违反下界、违反上界或可行的情况，<code class="math">a_e\in\{-1,0,1\}</code>。由于总违反量小于 1，任何违反坐标在盒内离其界限均小于一个单位，故这些仿射公式在整个盒内成立。

冻结整数坐标。维护一个由非整数边构成的森林。每条森林外边闭合一条基本环；选择使 <code class="math">a</code>-费用非正的方向，并在舍入盒内尽可能沿该环推送。至少有一条边变成整数。切断前，对暴露路径上的所有同时达到零的剩余最小值作标记并枚举，然后将其冻结并移除。若处理的边仍为非整数，则将其连入森林。自环对应单边环。

每条边至多被处理、连入和移除一次。剩余分数支撑是一片森林。若它非空，则必有叶子；该叶子除某个非整数关联外，其余关联均为整数，因而在其他坐标均为整数后其散度为非整数，与整数需求矛盾。故最终向量为整数。其违反量是非负整数，且不超过原来小于 1 的值，所以为零。推送量是二进制有理坐标与整数端点之差，不会引入新分母。∎

### 8.4 精确交换及其残余剥离

对整数账本及目标底数 <code class="math">c</code>，令

<!-- math-display: 8e4d4aaff3b60ba2840148d9
b_d=(cw_d-y_d)_+,\\qquad
 u_d=(y_d-cw_d)_+,\\qquad \\beta=\\sum_db_d.
-->
![公式](../../assets/math/8e4d4aaff3b60ba2840148d9.svg)

交换网络包含容量为 <code class="math">b_d</code> 的弧 <code class="math">s\to d</code>、容量为 <code class="math">u_d</code> 的弧 <code class="math">d\to t</code>；对每个关联 <code class="math">e=(a,d)</code>，还包含容量分别为 <code class="math">x_e</code> 的弧 <code class="math">d\to a</code>，以及容量为 <code class="math">L+1</code> 的弧 <code class="math">a\to d</code>。整数最大流 <code class="math">F</code> 给出

<!-- math-display: bc77f6bca08ba4a0d0d4ba1a
x'_e=x_e-F_{d,a}(e)+F_{a,d}(e),\\qquad
 y'_d=y_d+F_{s,d}-F_{d,t}.
 \\tag{27}
-->
![公式](../../assets/math/bc77f6bca08ba4a0d0d4ba1a.svg)

行流守恒保持行和为 <code class="math">L</code>；行到箱弧无法饱和其 <code class="math">L+1</code> 容量。若流值为 <code class="math">\beta</code>，则所有亏缺均被填满。

否则令 <code class="math">R</code> 为从 <code class="math">s</code> 出发的全部残余可达箱。箱 <code class="math">d</code> 到行 <code class="math">a</code> 的组合残余容量恰好是 <code class="math">x'_e</code>。可达行会到达其所有邻箱。因此恰好由受困行向 <code class="math">R</code> 提供分配，并有

<!-- math-display: b81bec0967dbd8fe73b244b7
y'(R)=L\\eta(R)<cw(R).
 \\tag{28}
-->
![公式](../../assets/math/b81bec0967dbd8fe73b244b7.svg)

在 <code class="math">R</code> 外，亏缺均被补足。删除 <code class="math">R</code> 及指派给其中的行后，限制新的分配；幸存行在 <code class="math">R</code> 中没有正分配。这正是第 7 节所需的零债务子状态。若冻结父状态满足底数 <code class="math">c+1</code>，则 <code class="math">\beta\leq P_{\mathrm d}</code>。其受困行还给出 <code class="math">y_{\rm parent}(R)\leq L\eta(R)</code>，因此
<code class="math">(c+1)w(R)\leq y_{\rm parent}(R)+p(R)\leq L\eta(R)+P_{\mathrm d}&lt;cw(R)+P_{\mathrm d}</code>。
故 <code class="math">w(R)&lt;P_{\mathrm d}</code>。

### 8.5 不再调用可行性求解器而恢复精确局部最大流

继承的局部阈值引擎使用少于 <code class="math">1000(P_{\mathrm d}+1)</code> 条普通弧。将容量乘以 16，令回边 <code class="math">t\to s</code> 的流量为 <code class="math">z</code>；对阈值 <code class="math">b</code> 设 <code class="math">T=16b</code>、回边权重 <code class="math">W=20000(P_{\mathrm d}+1)</code>。目标函数为

<!-- math-display: 32ca76f04dfd5675674c864b
\\Phi=\\sum_{\\text{ordinary }e}
  [\\log(F_e-l_e+\\zeta_e^-)+\\log(U_e+\\zeta_e^+-F_e)]
                  -W\\log(T-z).
 \\tag{29}
-->
![公式](../../assets/math/32ca76f04dfd5675674c864b.svg)

迭代点严格守恒。未加填充的普通违反量至多为 <code class="math">\alpha&lt;S^{-800}&lt;1/100</code>。正锁只出现在汇点弧上。未展开边界的流量始终小于 <code class="math">10u_d</code>，而其真实缩放汇容量为 <code class="math">16u_d</code>。SUCCESS 表示 <code class="math">z\geq16b-1/4</code>；LOW 满足排除强方向 (22) 的性质。

若成功，则丢弃锁，除以 16，安装真实普通容量，并将回边界设为 <code class="math">[b,\beta]</code>。总违反量至多为 <code class="math">1/64+\alpha/8&lt;1</code>。引理 8.1 返回一个真正的整数可行流，其流值恰为 <code class="math">b</code>，因为回边舍入区间的上端点为 <code class="math">b</code>。

对于紧邻最优值上方的最终 LOW 检验，写 <code class="math">b=v+1</code>，其中解锁后的乐观最大流值为 <code class="math">v</code>。由引理 8.1，锁定容量可行。一个可行锁定流可以在不降低汇点弧流量的情况下达到解锁后的最大值：简单的源汇残余增广不会使用从汇点出发的弧。这证明存在一个尊重锁的比较流，值为 <code class="math">16v</code>。

令 <code class="math">d=16v-z</code>。若 <code class="math">d\geq8</code>，回边导数为 <code class="math">Wd/(16+d)\geq W/3</code>，而所有普通负贡献的总量小于 <code class="math">2000(P_{\mathrm d}+1)</code>。带符号导数与绝对导数之比超过一半，与 LOW 矛盾。因此 <code class="math">z&gt;16v-8</code>。移除锁并除以 16 后，违反量小于 <code class="math">1/2+\alpha/8&lt;1</code>，且回边下界为 <code class="math">v</code>。舍入得到流值为 <code class="math">v</code> 的乐观最大流，残余可达性则给出其最小割。

运行 <code class="math">O(s)</code> 次阈值测试得到候选最优值 <code class="math">v</code>；保留或重跑在 <code class="math">v</code> 处的 SUCCESS 及 <code class="math">v+1</code> 处的 LOW，并对 <code class="math">0,\beta</code> 作自然的单侧处理。检查守恒、容量、整数性、流值及割容量。乐观边界不可能位于该割的源侧，因为其汇容量为 <code class="math">\beta+1&gt;v</code>。因此该割所有离开整个网络的弧都已被表示，它也是容量为 <code class="math">v</code> 的完整网络割。

要恢复完整残余剥离，将成功区域与失败区域按原弧标识取并，安装真实容量，并将真实成功流以零值扩展。其流值等于完整割容量，所以完整残余网络中没有弧离开该割。割内的正向残余弧出现在失败区域中；正流对应的反向弧出现在成功区域中。因此，并集中的可达性恰好等于完整残余可达性。所有残余搜索都忽略数值回边。

由此以 <code class="math">O(P_{\mathrm d}+1)</code> 条记录恢复式 (27) 及完整剥离。全扩展恢复使用同一论证，只需将 <code class="math">P_{\mathrm d}+1</code> 替换为实际图体积。

### 8.6 匹配客户端的完整成本

每个局部阈值目标值范围（包括插入与锁定损失）为 <code class="math">O((P_{\mathrm d}+1)s)</code>。进展量为 <code class="math">\Omega(q^2)</code>，故

<!-- math-display: e3ccf087a48413db70bb0a62
J_{\\rm step}=(P_{\\mathrm d}+1)D^2\\mathrm{poly}(s),
 \\qquad
 Q_{\\rm own}=(P_{\\mathrm d}+1)D^3\\mathrm{poly}(s).
-->
![公式](../../assets/math/e3ccf087a48413db70bb0a62.svg)

其名义生命周期至多为 <code class="math">8192(P_{\mathrm d}+1)</code>，向上取整到二的幂时取 <code class="math">M=\mathrm{pow2ceil}(16(R+2))</code>。由式 (25)、所有 <code class="math">O(s)</code> 次测试、舍入及最终恢复可得

<!-- math-display: b18d6691f9b5081f30db79ce
T_{\\rm projection}\\leq
       (P_{\\mathrm d}+1)D^4\\mathrm{poly}(s).
 \\tag{30}
-->
![公式](../../assets/math/b18d6691f9b5081f30db79ce.svg)

顶层客户端取 <code class="math">R_{\rm top}=S(s+1)^{10}</code>，成本为 <code class="math">SD^4\mathrm{poly}(s)</code>。根据式 (17)，局部生命周期为 <code class="math">O(sS)</code>；最终顶层及局部名义规模均小于 <code class="math">S^2</code>。根价格及逻辑容量界在选择 <code class="math">D,q</code> 之前固定；重新定价复用端点槽。

将式 (30) 用式 (18) 求和，加入 <code class="math">O(s)</code> 次全扩展重置与结构工作，再计入 <code class="math">O(s^2)</code> 个阶段。由于 <code class="math">D=O(E)</code>，匹配侧约简的完整成本为

<!-- math-display: b6cec42905288680f4259c87
T_{\\rm match}\\leq S E^4\\mathrm{poly}(s).
 \\tag{31}
-->
![公式](../../assets/math/b6cec42905288680f4259c87.svg)

先声明宽松的多项式操作、生命周期与迭代守卫。在其前缀上，微小填充改变量总和小于 <code class="math">S^{-800}</code>；更精细的目标值和局部性计数随后可推出，当 <code class="math">E=S^{o(1)}</code> 时最终不会失败。这样的顺序确保先建立精度前提，再使用最终快速时间界。

## 9. 带显式路径的顺序扩张子路由

该路由构造的优点在于：其输出质量对深度参数的依赖在线性地进入对数指数，而昂贵的局部构造则作为单独成本处理。

**命题 9.1（路由组装）。** 对一个连通、无自环、含 <code class="math">m'</code> 条边、导通率至少为 <code class="math">\phi&gt;0</code> 的多重图，若主对数规模库存上界为 <code class="math">U</code>，下文指定的层次结构与局部构造给出预处理工作量

<!-- math-display: 3c1b8f2a0222f42affb9aa8d
m'\\exp O(U/p+ph+\\log\\phi^{-1})\\mathrm{poly}(U).
 \\tag{32}
-->
![公式](../../assets/math/3c1b8f2a0222f42affb9aa8d.svg)

若每个顶点的起点和终点数均至多为 <code class="math">D_{\rm tok}\deg(v)</code>，则一次查询返回显式简单路径：路径长度至多为 <code class="math">Q</code>，边出现拥塞度至多为 <code class="math">D_{\rm tok}Q</code>，工作量至多为 <code class="math">m'D_{\rm tok}Q\mathrm{poly}(U)</code>，其中

<!-- math-display: 228a15b74ef61216c2f6e9b8
Q=\\exp O(ph+\\log\\phi^{-1}).
 \\tag{33}
-->
![公式](../../assets/math/228a15b74ef61216c2f6e9b8.svg)

多项式次数与常数均独立于 <code class="math">p</code>。几何层次结构使用 [CS, CHS] 的模块接口及第 11.6 节的实现要求。以下小节明确这些接口，并证明组合中所用的顺序替换及成本界。

### 9.1 关联端口与实际规模递归

为每个关联分配一个端口。度数为 <code class="math">d</code> 的顶点获得一个立方体纤维，其端口数为严格大于 <code class="math">d</code> 的最小 2 的幂；每条真实边连接其指定端口。基本立方体不等式

<!-- math-display: 95e1b1d044d1bd85c3a6e444
|\\partial X|\\geq\\min\\{|X|,|V(Q_a)\\setminus X|\\}
 \\tag{34}
-->
![公式](../../assets/math/95e1b1d044d1bd85c3a6e444.svg)

可由维数归纳推出：将割分到两个较小立方体及连接它们的匹配上即可。端口图至多有 <code class="math">4m'</code> 个顶点、<code class="math">O(m'\log(m'+2))</code> 条边，度数为 <code class="math">O(\log(m'+2))</code>。

对端口割，令 <code class="math">z</code> 为各纤维中少数侧端口数。立方体内部边界贡献至少 <code class="math">z</code>；与多数侧对应的原图顶点割比较，可知外部跨边至少为 <code class="math">c\phi\min\{&#124;X&#124;,&#124;\overline X&#124;\}-O(z)</code>，其中 <code class="math">c&gt;0</code> 为固定常数。因此端口图导通率至少为 <code class="math">\phi/O(\log(m'+2))</code>。这一损失只是关于 <code class="math">U</code> 的固定多项式因子。将起点与终点循环分配到真实关联端口；每个端口至多分得 <code class="math">D_{\rm tok}</code> 个起点或终点。

对实际端口图阶数 <code class="math">n'</code>，令 <code class="math">v=\lceil\log(n'+2)\rceil</code>，并定义

<!-- math-display: b33d2f01a7bb4c43ec23213f
b_{\\rm route}=2^{\\max\\{5,\\lceil v/p\\rceil\\}},\\qquad
 \\lambda_{\\rm cap}=C_\\lambda(v+1)^2.
 \\tag{35}
-->
![公式](../../assets/math/b33d2f01a7bb4c43ec23213f.svg)

当实际层次节点不超过 <code class="math">C_{\rm leaf}b_{\rm route}^5\lambda_{\rm cap}^2</code> 个顶点时停止递归，其中常数固定且足够大。这是叶节点测试，不是分配一个补齐后的填充图。深度为 <code class="math">O(p)</code>。真实内部划分满足 <code class="math">2b_{\rm route}/3\leq t_{\rm part}\leq b_{\rm route}</code>；每个部分的规模为 <code class="math">\Theta(&#124;X&#124;/b_{\rm route})</code)，且 <code class="math">n'_X=6&#124;X&#124;/b_{\rm route}</code>。特别地，<code class="math">t_{\rm part}\geq16</code>。各部分满足

<!-- math-display: afea5a684ed5ffa873c5cea6
\\min_i|Y_i|\\geq8\\lambda t_{\\rm part}^2,\\qquad
 n'_X\\geq\\lambda t_{\\rm part}^2,\\qquad
 t_{\\rm part}\\leq b_{\\rm route},\\quad \\lambda\\leq\\lambda_{\\rm cap}.
 \\tag{36}
-->
![公式](../../assets/math/afea5a684ed5ffa873c5cea6.svg)

这些整数条件将用于查询证明。

### 9.2 短路径、实际割与成功覆盖

局部匹配基本程序维护一棵截断到深度 <code class="math">d+2</code> 的广度优先搜索树，并加入超源点与超汇点。提取一条长度至多为 <code class="math">d</code> 的最短路径，删除其所有物理顶点，然后继续。距离只会上升；每条弧扫描 <code class="math">O(d+2)</code> 次。因此可在 <code class="math">O((n+m)(d+2))</code> 工作量内得到精确的短顶点不相交路径极大打包。对至多 <code class="math">b_{\rm route}</code> 种颜色顺序处理，会额外引入构造因子 <code class="math">b_{\rm route}</code>；但一轮中的路径并集仍保持顶点不相交。

当 <code class="math">d=O(\Delta\psi^{-1}\log(n+2))</code> 时，要么找到足够多短路径，要么距离球给出一个真实稀疏割。由于增长率乘积至多为 <code class="math">n</code>，某个半径处的相对增长为 <code class="math">O(\log n/d)</code>。临时隔离的路径顶点总边界至多为其数量乘以 <code class="math">\Delta</code>；小规模打包不等式支付这部分恢复边界。因此每个报告割确实是其声明的完整宿主图中的割。

重复匹配轮次使用完整且未变化的宿主图，只更新已匹配端点列表，并以空排除集开始。外层第 <code class="math">j</code> 轮使用阈值 <code class="math">\psi/(16j^2)</code>。可和的边界预算控制失败割的并集。精确的单色覆盖要么饱和所有剩余源点，要么返回一个包含所有未覆盖源点的真实割。因此所有剩余终端均有表示。

对当前 <code class="math">N</code> 顶点宿主图调用弱割／扩张子程序，会返回大小介于 <code class="math">\beta N</code> 与 <code class="math">N/2</code> 之间的割，或返回一个至少有 <code class="math">(1-\beta)N</code> 个顶点的虚拟扩张子，其中 <code class="math">\beta=1/1000</code>。谱叶只剪除小割，直至被删并集首次达到 <code class="math">\beta N</code>。若它改为认证扩张，则保留图同样覆盖至少 <code class="math">(1-\beta)N</code>。通过精确有理 Laplace 矩阵半正定检验及加权中位数扫描，可用固定多项式局部工作完成叶节点／粗割判定。

强包装器持续删去不相交的弱割，直到它们的并集首次达到 <code class="math">N_0/4</code>；或者保留最后一个弱成功结果。在割退出情形，被删并集小于 <code class="math">5N_0/8</code>；取补集造成的稀疏度损失小于 <code class="math">5/3</code>。在成功退出情形，覆盖率超过

<!-- math-display: 2357291bdecb7aeb1371ffa8
(1-\\beta)\\frac{3N_0}{4}>\\frac{2N_0}{3}.
 \\tag{37}
-->
![公式](../../assets/math/2357291bdecb7aeb1371ffa8.svg)

弱调用次数为 <code class="math">O(1/\beta)</code>。对于承诺为扩张子的根，每个实际宿主图阈值都严格小于真实稀疏度。因此首次删割不可能发生；覆盖必定覆盖整个根。首次粗略删除也不可能发生，因为其块边界正好是对应的原宿主割。根节点覆盖是精确的。

### 9.3 半匹配熵进展

**引理 9.2。** 设一个抽象 <code class="math">N</code> 顶点图是先前若干匹配的多重图并集。假设某个持续处理的割满足 <code class="math">N/4\leq &#124;S&#124;=a\leq N/2</code>，其边界至多为 <code class="math">a/8</code>。再加入一个大小至少为 <code class="math">a/2</code> 的、连接 <code class="math">S</code> 与其补集的匹配，并在匹配点对上惰性平均，则总转移熵至少增加 <code class="math">5N/1024</code>。

**证明。** 记 <code class="math">p(j,k)</code> 为此前的双随机转移矩阵。初始时在 <code class="math">S</code> 上放置单位质量；每个此前跨割的匹配出现项至多发送半单位。因此任一方向的跨割质量至多为 <code class="math">a/16</code>。

令 <code class="math">S_*</code> 为新匹配中的源端点集合。对 <code class="math">j\in S,k\in S_*</code> 的 <code class="math">p(j,k)</code> 总质量至少为 <code class="math">a/2-a/16=7a/16</code>。若匹配三元组满足 <code class="math">p(j,k)\geq2p(j,\ell(k))</code>，则称其为良好。坏三元组的源侧总质量至多为跨割质量的两倍，即 <code class="math">a/8</code>。因此良好质量至少为 <code class="math">5a/16\geq5N/64</code>。

对熵密度 <code class="math">H(z)=z\ln(1/z)</code>，若 <code class="math">w\leq u/2</code>，则将 <code class="math">u,w</code> 平均可获得至少 <code class="math">u/16</code> 的熵增。除以 <code class="math">u</code> 并对所得单变量函数求导可知，其在 <code class="math">w/u\in[0,1/2]</code> 上的最小值是 <code class="math">\frac12\ln(32/27)&gt;1/16</code>。其他所有配对由凹性带来非负增益。乘以良好质量下界即得结论。∎

熵位于 <code class="math">[0,N\ln N]</code>，故 <code class="math">512\lceil\log(N+2)\rceil</code> 轮持续迭代已绰绰有余。每次重复出现的匹配边在多重图中均保持为不同边。

### 9.4 虚拟层次结构与多项式局部构造

每个成功节点输出一个定义于其终端集 <code class="math">X</code> 上、恰好以该集合为顶点的显式图，并将其划分为

<!-- math-display: 169398c4abe473f8a3699375
X=\\dot\\bigcup_{i=1}^{t_{\\rm part}}Y_i,\\qquad
 Y_i=X_i\\dot\\cup B_i,
 \\tag{38}
-->
![公式](../../assets/math/169398c4abe473f8a3699375.svg)

其中一个单射覆盖匹配将 <code class="math">B_i</code> 映射到良好子节点 <code class="math">X_i</code>。子图和覆盖边按字面插入父虚拟图。特别地，每个良好子图都是根虚拟图中的诱导子图。路径内部顶点记录在嵌入中，不会扩大终端集。

保留的层次结构只有一个物理映射 <code class="math">f_0:H_{\rm root}\to G^\diamond</code>。其长度与拥塞分别为 <code class="math">\phi^{-O(1)}v^{O(p)}</code>。非根构造宿主是 <code class="math">O(v)</code> 个匹配的并集，度数为 <code class="math">O(v)</code>；成功对象会终止其博弈过程。每一层深度只产生关于 <code class="math">v</code> 的多项式扩张损失。因此，原始逆扩张因子只由原始根支付。在同一深度内，各良好节点的边库存互不相交。仅在保留最终层次结构后，才构造洗牌器与叶电路；将它们的基本路径并入虚拟根，最后只应用一次 <code class="math">f_0</code>。整个基本路径族具有共同的路径长度与总拥塞上界

<!-- math-display: 20fd54d46394f97dbe416a8f
A_{\\rm fam}=\\phi^{-O(1)}v^{O(p)}
 \\tag{39}
-->
![公式](../../assets/math/20fd54d46394f97dbe416a8f.svg)

下面给出其余局部计算的有限算法。

**能量割玩家。** 对有理数中心化行向量 <code class="math">v_i=R_i-\overline R</code>，令 <code class="math">\Phi=\sum_i\&#124;v_i\&#124;^2</code>，并令 <code class="math">a_0=\lceil\log_4(8t_{\rm part}^3)\rceil</code>。通过条件期望选择符号 <code class="math">z\in\{-1,1\}^{t_{\rm part}}</code>，以最大化

<!-- math-display: 2387023ace311638281ef8ae
\\Psi(z)=\\sum_i\\langle v_i,z\\rangle^2
 -t_{\\rm part}\\Phi\\sum_{i<j:v_i\\ne v_j}
 \\left(\\frac{\\langle v_i-v_j,z\\rangle^2}
 {8a_0\\|v_i-v_j\\|^2}\\right)^{a_0}.
 \\tag{40}
-->
![公式](../../assets/math/2387023ace311638281ef8ae.svg)

偶数阶矩不等式 <code class="math">\mathbb E\langle d,z\rangle^{2a_0}\leq(2a_0)^{a_0}\&#124;d\&#124;^{2a_0}</code> 给出 <code class="math">\mathbb E\Psi\geq7\Phi/8</code>。最大化条件期望可使投影能量至少为 <code class="math">7\Phi/8</code>，而任意一对投影距离平方至多是真实距离平方的 <code class="math">8a_0</code> 倍。将投影排序，以中位数为基准取顶部与底部八分位中较重的一侧，并与相反的一半配对。所得集合 <code class="math">T,T'</code> 对任意映射 <code class="math">\sigma:T\to T'</code> 都满足

<!-- math-display: bee119207224f56091412f27
\\sum_{i\\in T}\\|R_i-R_{\\sigma(i)}\\|^2\\geq\\frac{\\Phi}{512a_0}.
 \\tag{41}
-->
![公式](../../assets/math/bee119207224f56091412f27.svg)

对物理源簇做饱和操作，得到一个分数匹配矩阵，其源行质量至少为 <code class="math">1/18</code>。其惰性对称更新使 <code class="math">\Phi</code> 至少下降 <code class="math">\Phi/(18432a_0)</code>。因此，经过 <code class="math">\lambda=O(\log(t_{\rm part}+2)\log(n'+2))</code> 轮，随机矩阵乘积的每个矩阵元与均匀分布相差至多 <code class="math">1/(100t_{\rm part})</code>。

条件偶数阶矩通过递推式计算：

<!-- math-display: a08c3bfabdeb17585dd5b7c8
M'_j=\\sum_{\\substack{0\\leq z\\leq j\\\\z\\ {\\rm even}}}
                       \\binom jz d^z M_{j-z}.
 \\tag{42}
-->
![公式](../../assets/math/a08c3bfabdeb17585dd5b7c8.svg)

所需操作数量是固定多项式级别，数值为多项式位数的有理数。较大的临时整数计入构造成本。

**叶电路。** 对实际叶节点，其阶数为 <code class="math">a</code>、最大度数为 <code class="math">\Delta</code>、边扩张率为 <code class="math">\psi</code>。使用对称惰性随机游走：每条有向边分配概率 <code class="math">1/(2D_0)</code>，其中 <code class="math">D_0</code> 是大于 <code class="math">\Delta</code> 的 2 的幂。经过 <code class="math">T=O(\Delta^2\psi^{-2}\log(a+2))</code> 步，转移矩阵每个元素至少为 <code class="math">1/(2a)</code>。通过一个均匀中间点和两段条件游走来路由每个有界负载需求。每半段相对于普通游走的密度至多为 2。边的期望总遍历负载为 <code class="math">O(T/D_0)</code>。

通过逐次固定中间点和有向边，对势函数 <code class="math">\sum_e(1+1/(2T))^{N_e}</code> 做条件期望去随机化。其精确条件矩为矩阵幂，其中该边的转移概率乘以 <code class="math">1+1/(2T)</code>。这给出长度至多 <code class="math">2T</code>、拥塞为 <code class="math">O(T\log(a+\Delta+2))</code> 的显式路径，所需位运算为固定多项式级别。在位序网络的 <code class="math">O(\log^2 a)</code> 个跨块层上预计算这些路径。对于负载 <code class="math">L_{\rm tok}</code>，将块长向上取整到 2 的幂。块内比较器距离保持局部；跨块模式不依赖块长。一次查询使用 <code class="math">O(aL_{\rm tok})</code> 个实际槽位，并对其算术运算及轨迹记录计费。

最后，层次结构构造满足实际规模递推

<!-- math-display: f2c3ab88cd032b8df085fec2
T(N,j)\\leq v^{c_1}\\sum_iT(N_i,j-1)
       +N b_{\\rm route}^{c_2}v^{c_3}
                      \\mathrm{poly}(\\psi_{\\rm cut}^{-1}),
 \\quad \\sum_iN_i\\leq c_4N,
 \\tag{43}
-->
![公式](../../assets/math/f2c3ab88cd032b8df085fec2.svg)

其中各常数均绝对固定。关于 <code class="math">b_{\rm route}</code> 的局部多项式工作是可加的；只有关于 <code class="math">v</code> 的多项式尝试因子会在 <code class="math">O(p)</code> 层上相乘。原始逆扩张率只在第一个宿主中出现；非根阈值则是有界度匹配宿主上的固定常数。连同叶节点与洗牌器计算，即得式 (32)。这是包含临时位运算与显式列表的顺序工作量论证。

### 9.5 散布、会合、反向与精确目的地

查询归纳使用保留的层次结构、洗牌器矩阵、式 (36) 与式 (39)。若要在部分 <code class="math">Y_i</code> 内路由，则沿其单射覆盖匹配将 <code class="math">B_i</code> 折叠进 <code class="math">X_i</code>，调用较小的子路由器，再将目的地展开。每个折叠纤维大小至多为 2。

对颜色 <code class="math">\ell</code>，令 <code class="math">N_{i\ell}</code> 为其在部分 <code class="math">i</code> 中的 token 数。洗牌器第 <code class="math">a</code> 步使用 <code class="math">p_{ij}^a</code> 条实际匹配门户连接部分 <code class="math">i,j</code>，从 <code class="math">i</code> 向 <code class="math">j</code> 发送恰好

<!-- math-display: 43f2a5f96513449fb3d8c12d
\\left\\lfloor\\frac{p_{ij}^a N_{i\\ell}}{2n'_X}\\right\\rfloor
 \\tag{44}
-->
![公式](../../assets/math/43f2a5f96513449fb3d8c12d.svg)

个 token。将它们循环分配到真实门户，并调用各部分路由器到达门户。对每种颜色，累计取整误差至多为 <code class="math">\lambda t_{\rm part}</code>。经过洗牌后，每部分的该颜色质量位于
<code class="math">.99N_\ell/t_{\rm part}-\lambda t_{\rm part}</code> 与
<code class="math">1.01N_\ell/t_{\rm part}+\lambda t_{\rm part}</code> 之间。
单个门户的总配额为 <code class="math">O(L_{\rm tok})</code>；散布期间顶点端点负载为 <code class="math">O(L_{\rm tok}(\lambda+1))</code>。

按目的部分给每个真实 token 染色。另行从每个顶点散布 <code class="math">2L_{\rm tok}</code> 个 dummy token，并按其来源部分染色。在每个当前部分和颜色中，dummy 数量都多于真实 token，因为根据式 (36)，

<!-- math-display: 378c1fa373fe6123853f04a1
.97L_{\\rm tok}|Y_j|/t_{\\rm part}
                       -2\\lambda t_{\\rm part}>0
 \\tag{45}
-->
![公式](../../assets/math/378c1fa373fe6123853f04a1.svg)

将真实 token 单射匹配到同一当前部分、同一颜色的 dummy。将真实 token 路由到这些 dummy 所在位置，再反向执行记录的 dummy 轨迹。它们以至多 <code class="math">2L_{\rm tok}</code> 的负载抵达正确部分。最后调用一次部分路由器到达精确目的地。

所有递归都进入严格更小的子节点。令 <code class="math">\mathcal V_j</code> 为深度 <code class="math">j</code> 的各调用中“顶点数乘以提供负载”的总和，则

<!-- math-display: 6a867082921bcf43b7a826fa
\\mathcal V_0=n'L_{\\rm tok},\\qquad
 \\mathcal V_{j+1}\\leq c(\\lambda+1)^2\\mathcal V_j.
 \\tag{46}
-->
![公式](../../assets/math/6a867082921bcf43b7a826fa.svg)

同一因子也界定每条存储基本路径的使用次数及重复轨迹消耗。因此，膨胀率至多为 <code class="math">A_{\rm fam}(c(\lambda+1))^{d+1}</code>，拥塞至多为 <code class="math">L_{\rm tok}A_{\rm fam}(c(\lambda+1)^2)^{d+1}</code>，工作量至多为 <code class="math">n'L_{\rm tok}</code> 乘以上述不含负载的因子及对数级簿记成本。此处 <code class="math">d=O(p)</code>。为枚举配额，按 <code class="math">p_{ij}^a</code> 降序存储门户组；每个正配额会消耗一个 token，每个非空部分／颜色另有一次停止测试。每一步工作量为 <code class="math">O((M_{\rm tok}+t_{\rm part}^2)\mathrm{polylog}(n'L_{\rm tok}+2))</code>，由式 (36) 支付。

将立方体端口移动投影回原宿主图，同时保留真实边标识；随后利用当前宿主图的生命周期 ID 删除闭合子游走。长度和所有负载只会减少。式 (39) 与式 (46) 给出命题 9.1 的式 (33) 以及完整查询量界。

## 10. 归一化静态稀疏化器

**命题 10.1（静态提供器）。** 设无自环需求多重图 <code class="math">J_0</code> 具有原生边标识、最大度数 <code class="math">\Delta_0</code>，并且每条边 <code class="math">e</code> 有一条保端点的环境游走 <code class="math">\Pi(e)</code>，最大长度为 <code class="math">L_0\geq1</code>，顶点出现拥塞度为 <code class="math">C_0</code>。对于 <code class="math">1\leq p,r\leq U</code> 且 <code class="math">(p+r)h\leq U</code>，下述构造返回归一化路径表 <code class="math">\Pi'</code>、所选子图 <code class="math">\widetilde J_0\subseteq J_0</code> 以及路由 <code class="math">R:J_0\to\widetilde J_0</code>，满足

<!-- math-display: a7720363e821e046bbe15d4b
\\begin{aligned}
 |E(\\widetilde J_0)|&\\leq |V(J_0)|\\Gamma\\mathrm{poly}(U),\\\\
 L(R)&\\leq\\Gamma,\\qquad L(\\Pi'\\circ R)\\leq\\Gamma L_0,\\\\
 C(\\Pi'\\circ R)&\\leq\\Gamma(C_0+\\Delta_0),
\\end{aligned}
 \\tag{47}
-->
![公式](../../assets/math/a7720363e821e046bbe15d4b.svg)

其中

<!-- math-display: 80eae4a81ff9800391fd1c07
\\log\\Gamma=O((p+r)h),\\qquad
 \\log F=O(U/p+U/r+r^2h+(p+r)h).
 \\tag{48}
-->
![公式](../../assets/math/80eae4a81ff9800391fd1c07.svg)

完整工作量以及实际顶点簿记为

<!-- math-display: 51f9577993ad4ca9971763ed
|E(J_0)|F^{O(1)}\\Gamma^{O(1)}(L_0+1)\\mathrm{poly}(U).
 \\tag{49}
-->
![公式](../../assets/math/51f9577993ad4ca9971763ed.svg)

该命题以命题 9.1 与继承的度数分带基本工具为前提。证明如下。

### 10.1 度数分带与显式加权见证

首先，使用当前环境的生命周期标识符，对每一条环境映像分别删除环。得到简单路径 <code class="math">\Pi'(e)</code>，长度至多为 <code class="math">L_0</code>，拥塞度至多为 <code class="math">C_0</code>。

参数为 <code class="math">r</code> 的度数分带基本工具将需求边划分为一个至多有 <code class="math">&#124;V(J_0)&#124;</code> 条边的残余图以及分带图 <code class="math">J_i</code>。其非平凡连通分量的导通率至少为 <code class="math">\phi=\exp(-O(rh))</code>，最小度数至少为 <code class="math">\rho_i=\phi2^i</code>，且 <code class="math">\phi\leq\rho_i\leq\Delta_0</code>。工具的工作量为 <code class="math">m_0\exp O(U/r+r^2h+rh)\mathrm{poly}(U)</code>，其中 <code class="math">m_0=&#124;E(J_0)&#124;</code>。

为清楚起见，这里从外部割／剪枝基本工具重建所需的约简。初始设 <code class="math">\ell=\max\{0,\lceil\log(2m_0/n_0)\rceil\}</code>，其中 <code class="math">n_0</code> 为非孤立顶点数，并维持不变量 <code class="math">&#124;E(G_i)&#124;\leq2^in_0</code>。以固定跨边比例分解 <code class="math">G_i</code>；在每个连通分量内，使用共同 <code class="math">\phi</code> 的一个足够小的固定缩放值，剪除度数低于 <code class="math">\phi2^i</code> 的顶点。剪枝体积界及分量丢弃预算使下一阶段至多剩下 <code class="math">2^{i-1}n_0</code> 条边。保留分量满足上述最小度数与导通率。对 <code class="math">\sum_{i\leq\ell}2^in_0=O(m_0)</code> 求和，即可支付实际扫描工作。分带数为 <code class="math">O(U)</code>，每个保留分带至多有 <code class="math">2^in_0</code> 条边。在较小图上的调用会将 <code class="math">r</code> 截断到来源允许的范围；这会提高扩张率，并仍落在主工作量包络之内。

对每个分量，给顶点 <code class="math">v</code> 赋权 <code class="math">w_v=\deg_{J_i}(v)/\rho_i</code>。权重至少为 1，且平衡：任何一个权重都不超过其余权重之和。将 <code class="math">\lceil w_v\rceil</code> 向上调整为若干组大小，使组大小介于该值与其两倍之间，组大小总和为不小于总量的最小 2 的幂，并确保每组至多占总量一半。线性容量填充扫描即可完成。将这些组收缩到立方体上，并删除组内立方体边。由式 (34)，所得无自环多重图 <code class="math">W_i</code>（所有边的权重均为 <code class="math">\rho_i</code>）满足

<!-- math-display: 7465c8fb32c1f43f2d8b3491
\\deg_{J_i}(v)\\leq\\rho_i\\deg_{W_i}(v)
                 \\leq O(U)\\deg_{J_i}(v),
 \\qquad \\Phi(W_i)\\geq1/O(U).
 \\tag{50}
-->
![公式](../../assets/math/7465c8fb32c1f43f2d8b3491.svg)

度数分带库存界加上显式立方体因子，给出

<!-- math-display: 0897e7b9fe283d97aba16bc4
\\begin{aligned}
 \\sum_i|E(W_i)|&\\leq O(|V(J_0)|\\phi^{-1}U^2),\\\\
 \\sum_i|E(W_i)|&\\leq O(m_0\\phi^{-1}U),\\\\
 \\sum_i\\rho_i|E(W_i)|&\\leq O(m_0U).
\\end{aligned}
 \\tag{51}
-->
![公式](../../assets/math/0897e7b9fe283d97aba16bc4.svg)

所有宿主图与见证均按实际库存生成。

### 10.2 两个路由方向都按完整查询量计费

对每条见证边，通过 <code class="math">J_i</code> 路由 <code class="math">\lceil\rho_i\rceil</code> 个单位 token 副本。由式 (50)，相对于宿主度数的端点负载为 <code class="math">O(U/\phi)</code>。总查询量为 <code class="math">O(m_0U/\phi)</code>。

反向路由时，对每条 <code class="math">J_i</code> 边经 <code class="math">W_i</code> 路由一个 token，其相对度数负载为 <code class="math">D_i=\max\{1,\lceil\rho_i\rceil\}</code>。由式 (51)，

<!-- math-display: 4f9bdb229d404c3a4e90f78e
\\sum_i|E(W_i)|D_i
 \\leq\\sum_i(\\rho_i+1)|E(W_i)|
 =O(m_0U/\\phi).
 \\tag{52}
-->
![公式](../../assets/math/4f9bdb229d404c3a4e90f78e.svg)

每个实际 <code class="math">J_i</code> 与 <code class="math">W_i</code> 连通分量只预处理一次。只有查询工作量才会乘上负载。命题 9.1 给出共同简单路径质量 <code class="math">Q=\exp O((p+r)h)</code>，以及式 (48) 中的工作因子。在各自当前宿主内删除这些 token 路径的环，和规范化 <code class="math">\Pi</code> 是两个独立步骤。

### 10.3 稀疏的确定性加权路径选择

**引理 10.2。** 设组 <code class="math">g</code> 有 <code class="math">q_g\geq1</code> 个显式选项，每个选项具有非负整数资源重数 <code class="math">a_{gjx}</code>。显式列表规模 <code class="math">S_{\rm list}</code> 对所有选项头及每个单位资源出现项计数；聚合重数仍按其出现次数预算计费。令

<!-- math-display: fa44eba4652cb3935835692a
\\mu_x=\\sum_g\\sum_{j=1}^{q_g}a_{gjx},\\qquad
 B_0=\\max_{g,j,x}q_g a_{gjx}.
-->
![公式](../../assets/math/fa44eba4652cb3935835692a.svg)

可以在显式列表规模乘以多对数级算术因子的时间内，确定性选取 <code class="math">j(g)</code>，使得

<!-- math-display: aa594307aa8436feb996a3a1
\\sum_g q_g a_{g,j(g),x}
 \\leq4\\mu_x+4B_0\\ln(2|X|+2)
 \\tag{53}
-->
![公式](../../assets/math/aa594307aa8436feb996a3a1.svg)

对每个使用到的资源均成立。将权重 <code class="math">q_g</code> 替换为任意 <code class="math">0&lt;w_g\leq q_g</code> 时，同样的界仍成立。

**证明。** 若 <code class="math">B_0=0</code>，任意选择即可。否则，对尚未处理的组定义

<!-- math-display: 86258c06adb09c71a8faee91
E_{gx}=\\frac1{q_g}\\sum_j e^{q_ga_{gjx}/B_0},\\qquad
 W_x=e^{(b_x-2\\mu_x)/B_0}\\prod_{\\rm unprocessed\\ g}E_{gx}.
-->
![公式](../../assets/math/86258c06adb09c71a8faee91.svg)

每个单独指数均位于 <code class="math">[0,1]</code>。由 <code class="math">e^u\leq1+(e-1)u</code> 可得初始时 <code class="math">W_x\leq1</code>。对一个选项取平均不会改变总势函数。为稀疏地选择使势函数最小的选项，忽略共同项，只需最小化

<!-- math-display: 06b9da78ba5bddb9cc8bfb5f
\\sum_{x:a_{gjx}>0}\\frac{W_x}{E_{gx}}
                                  (e^{q_ga_{gjx}/B_0}-1).
 \\tag{54}
-->
![公式](../../assets/math/06b9da78ba5bddb9cc8bfb5f.svg)

所有项均非负，只更新该组出现的资源。因此每个资源出现项只会被处理多对数次。精确期望使总势函数不超过 <code class="math">&#124;X&#124;</code>，由此得到更强的界 <code class="math">2\mu_x+B_0\ln&#124;X&#124;</code>。

有限精度实现可对正指数级数及正数和使用固定倍数于 <code class="math">\log(S_{\rm list}+2)</code> 的尾数位数，并显式存储二进制指数。基本因子中的指数均处于 <code class="math">[0,1]</code>；累计权重的二进制指数只需 <code class="math">O(\log(S_{\rm list}+2))</code> 位。对每个基本计算取相对误差 <code class="math">(S_{\rm list}+2)^{-c}</code>，其中 <code class="math">c</code> 为足够大的固定常数。近似最小值选择使真实势函数每组最多增加 <code class="math">1+O(1/(S_{\rm list}+1))</code> 因子；总增加有界，可吸收到式 (53) 中。用正项级数计算 <code class="math">e^u-1</code> 可避免相减消去。∎

将此引理应用于每条见证边的正向候选路径，取 <code class="math">q_g=\lceil\rho_i\rceil</code>、权重 <code class="math">\rho_i</code>。与 <code class="math">\Pi'</code> 组合后，候选总资源质量为 <code class="math">O(UQC_0/\phi)</code>。每个候选最多使用 <code class="math">Q</code> 条宿主边，每条边的环境映像是简单路径，因此候选在任意环境顶点处的重数为 <code class="math">O(Q+1)</code>，与 <code class="math">L_0</code> 无关。因此

<!-- math-display: de77c6ce44f79a6e7f18ca67
B_0\\leq O((\\Delta_0+1)(Q+1)).
-->
![公式](../../assets/math/de77c6ce44f79a6e7f18ca67.svg)

所选见证路径的加权环境拥塞至多为

<!-- math-display: de75b7017c131569b4fb6bec
Q\\phi^{-O(1)}\\mathrm{poly}(U)(C_0+\\Delta_0).
 \\tag{55}
-->
![公式](../../assets/math/de75b7017c131569b4fb6bec.svg)

对数只涉及已使用的环境资源；其对数级库存规模为 <code class="math">O(U)</code>。

### 10.4 所选输入支撑与组合

保留所选见证路径使用的所有原生 <code class="math">J_0</code> 边标识，并加入残余边。由式 (51)，这些边的数量至多为 <code class="math">&#124;V(J_0)&#124;Q\phi^{-O(1)}\mathrm{poly}(U)</code>。

每个反向路由对一条见证边的拥塞至多为 <code class="math">D_iQ</code>；按权重 <code class="math">\rho_i</code> 归一后至多为 <code class="math">2Q/\phi</code>。将每个见证出现项替换为其选定的正向路径。这使式 (55) 至多再乘以 <code class="math">2Q/\phi</code>；最终宿主路径长度至多为 <code class="math">Q^2</code>。这些固定次幂给出式 (47)–(48)。扫描所有组合后的出现项列表会产生式 (49) 中的 <code class="math">L_0+1</code> 因子。因此环境输入长度影响构造工作量，而拥塞界只以 <code class="math">C_0+\Delta_0</code> 的加性形式依赖初始拥塞。

### 10.5 归一化后的精确修正环流

令 <code class="math">R(e)</code> 为边 <code class="math">e</code> 在当前所选支撑上的路由；它与 <code class="math">e</code> 具有相同的当前端点，原始加权长度至多为 <code class="math">\delta_{\rm str}\ell_e</code>。所选边保留自身的原生长度和梯度。定义修正量 <code class="math">d_e=e-R(e)</code>。对每个环流 <code class="math">c</code>，有

<!-- math-display: 6605f10a6d4fc6d41d6c05da
c=\\sum_e c_e(e-R(e))+R(c).
 \\tag{56}
-->
![公式](../../assets/math/6605f10a6d4fc6d41d6c05da.svg)

每一项都是其相应图中的环流。其原始分母收费总量至多为

<!-- math-display: 0cf953b0921c0d4631b072f4
(1+2\\delta_{\\rm str})\\sum_e\\ell_e|c_e|.
 \\tag{57}
-->
![公式](../../assets/math/0cf953b0921c0d4631b072f4.svg)

对分子使用线性性并作加权平均，可得某个修正环流或所选图环流，其比率至少为比较方向比率除以 <code class="math">1+2\delta_{\rm str}</code>。这是存在性／比较论证；算法会维护相应备选项。

将路由 <code class="math">R_0(e)</code> 替换为 <code class="math">R_1(e)</code>，会使修正量精确改变为环流 <code class="math">R_0(e)-R_1(e)</code>。归一化造成的全部梯度偏差均保留在新的 <code class="math">e-R_1(e)</code> 中。即使几何上的删环操作从映像中删除了出现项，投影边仍将其中心原始标识计入支撑。不同别名在显式指定类型的投影之前仍保持彼此独立。这些约定保留式 (56) 与原始提升性质。

## 11. 动态修复、度数重置与完整原始后端

本节构造动态环搜索层次结构，并记录其按时间顺序计的工作量、路径的字面长度以及有限精度算术。第 11.1–11.5 节给出主要不变量与递推。第 11.6 定理指出实现各项所需接口的完整配套构造，并证明这些构造组合后满足相应界。

### 11.1 实际规模的稳定核心

每个非基底父图的最大度数为三。一个新周期从其实际非孤立图开始，居民规模为 <code class="math">m'</code>，持续 <code class="math">O(m'/k)</code> 次父层编辑。在周期内，新变为孤立的槽位仍保留并计数。后续周期从已维护的非孤立顶点列表开始，不重新扫描无关的私有孤立点。

确定性中心接口使用 <code class="math">O((n'/k)\log^2(n'+2))</code> 个大连通分量中心、大小至多为 <code class="math">k</code> 的严格球，以及大小至多为 <code class="math">2k</code> 的逆簇；工作量为 <code class="math">O(n'k\mathrm{polylog}(n'+2))</code>。该接口通过最近顶点列表的贪心命中集得到；随后反复命中剩余超大逆簇中的前缀，每轮至少使一半超大顶点稳定。较小连通分量共用一个局部中心。

稳定森林的每个分量包含 <code class="math">O(k)</code> 个物理顶点。端点提升与单调割保留初始的未来路径库存；新生成的中间边另行记录。对未来路径加权后反复二分，可得到关于 <code class="math">U</code> 的多项式规模森林族。令 <code class="math">A=\mathrm{poly}(U)</code>，用以涵盖森林数量、桶、割日志及索引因子。则：

- 原生核心度数至多为 <code class="math">Ak</code>；
- 初始相关的非自环根数至多为 <code class="math">Am'/k</code>；
- 一次父层编辑或请求的反馈割至多产生 <code class="math">A</code> 次原生核心编辑；
- 原生分裂只是划分已有输入关联，保留其边生命周期 ID。

整个小连通分量树仍留在父层存储与物理池计数中。只有被选中的非自环端点才向下递归发布。已登记的自环需求保留其当前端点注记和空路由。若一次原生分裂使其变成非自环，则会触及两个端点；这不会创建一批新的输入生命周期。

### 11.2 触及集修复与所选边反馈

选取

<!-- math-display: 8a911998fd88cf806315668b
\\rho=2^{\\lceil8U/K_{\\mathrm r}\\rceil},\\qquad
 N_{\\rm sched}=\\rho^{K_{\\mathrm r}}.
 \\tag{58}
-->
![公式](../../assets/math/8a911998fd88cf806315668b.svg)

后者是一个计数器，不是分配出来的全集。使用命题 10.1 初始化稀疏静态基底。真实的新边生命周期通过恒等路由进入基底。修复层从空状态开始，第 <code class="math">j</code> 层周期为 <code class="math">\rho^{K_{\mathrm r}-j}</code>；原生删除或分裂会使时钟增加。到达周期边界时，重建最小的合资格层，并清空所有更高层。

触及集调度遵循 [CKLMPG，第 7.2 节] 的局部模板；本文对其静态基底、归一化路径、度数反馈及按时间顺序的额度记账作出具体规定。

路由有效性的归纳从首次被触及的顶点开始：在其最近一次相关重建或创建时，该需求已有有效的低层路由。首次触及顶点之前的前缀仍然有效。修复过程在这些顶点之间对投影后的中间需求作稀疏化。被选中的投影边只将其中间的原生边插入支撑；其前缀与后缀已经位于保留的低层中。

周期为 <code class="math">a</code> 的层，在发生 <code class="math">t</code> 次原生事件后重建时，至多触及 <code class="math">O(\min\{t,\rho a\})</code> 个顶点。因此，在该前缀中所有被选成员的出生总数至多为 <code class="math">A\Gamma\rho t</code>。对至多 <code class="math">U+1</code> 层求和并将该因子吸收到 <code class="math">A</code> 中，得

<!-- math-display: 8db2b12e2fadf3c1c191ee39
I_{\\rm repair}(t)\\leq \\alpha t,\\qquad
 \\alpha=A\\Gamma\\rho.
 \\tag{59}
-->
![公式](../../assets/math/8db2b12e2fadf3c1c191ee39.svg)

唯一的初始所选额度为 <code class="math">I_0\leq A\Gamma m'/k</code>。

在度数反馈中，只有当所选星的度数超过 <code class="math">6z</code> 时才对其分区，其中 <code class="math">z=\lceil64A\alpha\rceil</code>。树分区基本工具 [KMPG，定理 3.8] 将其物理原像切成若干片，使每片的所选度数至多为 <code class="math">3z</code>；除一片外，其余各片至少有 <code class="math">z</code> 条关联。使用势函数

<!-- math-display: 0a54b04fb672e92a40dbf9cc
\\Phi_{\\rm deg}=\\sum_v(\\deg_{\\rm selected}(v)-z)_+.
-->
![公式](../../assets/math/0a54b04fb672e92a40dbf9cc.svg)

一次所选边插入最多使其增加 2；删除与分裂不会使其增加。若分为 <code class="math">a\geq3</code> 片，则势函数至少下降 <code class="math">z(a-1)/2</code>。令 <code class="math">f</code> 为请求反馈割数，<code class="math">q_{\rm par}</code> 为父编辑数。由于

<!-- math-display: 07b747abed22a4157bbad4ea
t\\leq A(q_{\\rm par}+f),\\qquad
 I\\leq I_0+Aq_{\\rm par}+A\\alpha(q_{\\rm par}+f),
-->
![公式](../../assets/math/07b747abed22a4157bbad4ea.svg)

不等式 <code class="math">zf/2\leq2I</code> 可以吸收反馈项，并在每个有限级联前缀上推出

<!-- math-display: 0c7b581b7fa75716af0c8dee
f=O(I_0/z+q_{\\rm par}),\\qquad
 I=O(I_0+A\\alpha q_{\\rm par}).
 \\tag{60}
-->
![公式](../../assets/math/0c7b581b7fa75716af0c8dee.svg)

这些前缀界证明级联必然终止。在计算度数或将权重传给物理树分区之前，必须先对不同所有者持有的中心标识去重。

反馈稳定后才发布子层轨迹。持续存在的所选边按其旧端点与最终端点比较。未发布的短暂出生后又死亡，不会向子层产生边。所得所选度数以及完整编码编辑因子 <code class="math">R_{\rm sel}</code> 满足

<!-- math-display: e3551dc438a611aa68203242
\\log R_{\\rm sel}=O(U/K_{\\mathrm r}+\\log\\Gamma+\\log U),
 \\tag{61}
-->
![公式](../../assets/math/e3551dc438a611aa68203242.svg)

初始规模和按时间顺序的子层轨迹至多为
<code class="math">O(m'R_{\rm sel}/k+R_{\rm sel}q_{\rm par})</code>。
加性的初始项是在该周期中的一笔额度，可以延后使用。

每次修复时，投影图的预备度数和环境拥塞均为 <code class="math">O(C_j+\Delta_{\rm native})</code>，其中 <code class="math">\Delta_{\rm native}=k\mathrm{poly}(U)</code>。这里使用了实际归一化路由所有者及其首次触及端点。命题 10.1 给出

<!-- math-display: b2f3acc5f40d7a09c0eadab4
L_{j+1}\\leq c\\Gamma L_j,\\qquad
 C_{j+1}\\leq c\\Gamma(C_j+k\\mathrm{poly}(U)).
 \\tag{62}
-->
![公式](../../assets/math/b2f3acc5f40d7a09c0eadab4.svg)

因此 <code class="math">L_{K_{\mathrm r}}\leq\Gamma^{O(K_{\mathrm r})}</code>，且
<code class="math">C_{K_{\mathrm r}}\leq k\mathrm{poly}(U)\Gamma^{O(K_{\mathrm r})}</code>。
完整路由列表、逆向出现项索引、迁移关联、原生端点变更及修正分数更新，均属于局部工作，且有界于

<!-- math-display: c54205a8467fe6c30de0a178
W=k^{O(1)}\\rho^{O(1)}F^{O(1)}
                       \\Gamma^{O(K_{\\mathrm r})}\\mathrm{poly}(U).
 \\tag{63}
-->
![公式](../../assets/math/c54205a8467fe6c30de0a178.svg)

只有真实的所选边来源信息与标签变化才作为递归输入编辑向下传播。更新修正路由只会根据式 (56) 改变修正记录，不会替换所选边的原生梯度。

### 11.3 固定的三度纤维与精确环比较

用正确的冻结规范识别相同物理根后，令稳定的所选图最大度数至多为 <code class="math">C_{\rm port}</code>，其中 <code class="math">C_{\rm port}</code> 是 <code class="math">R_{\rm sel}</code> 的固定二进制上界。给每个实际所选顶点生命周期分配一个固定满二叉树，叶子数为 <code class="math">C_{\rm port}</code>，并将当前关联分别指派给不同叶子。对每条所选边，在其指派叶子间添加一条真实子边。令

<!-- math-display: 9decdb5e91e1ca03aa7893ae
H_{\\rm port}=
 2^{\\lceil\\log(4(\\log C_{\\rm port}+1))\\rceil}.
 \\tag{64}
-->
![公式](../../assets/math/9decdb5e91e1ca03aa7893ae.svg)

真实边的长度和梯度同时乘以 <code class="math">H_{\rm port}</code>；纤维连接边长度为 1、梯度为 0。子图最大度数至多为三。在所选度数暂时为零期间复用同一个纤维；只有旧生命周期完全退役后才回收关联叶子。已删除的父生命周期会留下一个计数中的非活跃纤维，直到外层周期丢弃。

一条简单所选图环每条真实边至多使用 <code class="math">2\log C_{\rm port}</code> 次连接边。由于真实边长度至少为 1，其子图提升长度至多为原缩放长度的两倍，梯度则恰好等于缩放后的梯度。因此环质量最多损失因子 2。反过来，收缩纤维会将每个子图环流映为所选图上的环流。纯纤维环流为零，因为纤维是树。

若子图答案的分子为 <code class="math">N_{\rm ch}</code>、原始证书为 <code class="math">V_{\rm ch}</code>，则其父图映像满足

<!-- math-display: b9d527bccc66c6dd01c2048a
N_{\\rm par}=N_{\\rm ch}/H_{\\rm port},\\qquad
 L_{\\rm raw,par}\\leq V_{\\rm ch}/H_{\\rm port}.
 \\tag{65}
-->
![公式](../../assets/math/b9d527bccc66c6dd01c2048a.svg)

路径系数仍为带符号的单位整数。所选边的梯度是其两段森林路径与中间父边梯度的精确值。冻结势能修正会在完整环流上望远镜消去。稳定森林分裂本身不会触发重新计算。即时空连接拥有各自带类型的来源信息；若一条边直到后续投影才变为空边，则它在其当前图中仍是真实边。

### 11.4 扁平出现项池与按时间顺序的递推

对每个父根 <code class="math">v</code>，令 <code class="math">T_v</code> 为其稳定森林树的直和，仅识别这些树的根。若子层每个顶点有 <code class="math">G_{\rm ch}</code> 个出现项，则为根 <code class="math">v</code> 预留 <code class="math">O(C_{\rm port}(G_{\rm ch}+1))</code> 个副本 <code class="math">T_{v,a}</code>，并在所有根间共用副本索引。将子层中每个出现在 <code class="math">v</code> 的某个端口上的出现项指派给不同索引。

真实子森林边通过一条连接相应池副本、位于适当森林位置的中心父边来提升。连接边或即时空边子边则成为带类型的根到根连接。收缩所有被指派的池树即可还原子森林，故提升结果仍是森林。未指派的副本是单独计数的树。

子层出现项的创建只占用现有池槽；子森林边变化只修改一条连接记录。它们不会复制整棵 <code class="math">O(k)</code> 顶点的父树。父稳定森林的割有不同收费方式：按出现项因子，对当前后代副本应用割日志。非活跃副本仍计入；退役副本在任何旧映像过期前，都会转移带符号增量和绝对移动量。池的扩容依据观测到的实际计数；所有替换与退役工作均计费。

将 <code class="math">R_{\rm sel}</code> 乘入固定多项式的编码与纤维因子，记为 <code class="math">P_{\mathrm b}</code>，则

<!-- math-display: 5d61086a7cfa2a5dbedb2519
\\log P_{\\mathrm b}
 =O(U/K_{\\mathrm r}+(p+r)h+h).
 \\tag{66}
-->
![公式](../../assets/math/5d61086a7cfa2a5dbedb2519.svg)

对剩余递归层数 <code class="math">d</code>，令 <code class="math">G_d</code> 为出现项数与答案片段数的上界，<code class="math">J_d</code> 为每条实际记录的初始工作额度，<code class="math">T_d</code> 为摊还更新工作量。一个安全的递推为

<!-- math-display: 37f002bf86e2a4bc307bd08c
\\begin{aligned}
 G_d&\\leq P_{\\mathrm b}(G_{d-1}+1),\\\\
 J_d&\\leq W+P_{\\mathrm b}(G_{d-1}+1)
        +(P_{\\mathrm b}/k)(J_{d-1}+T_{d-1}),\\\\
 T_d&\\leq O(kW+kP_{\\mathrm b}(G_{d-1}+1))
                 +O(P_{\\mathrm b})(J_{d-1}+T_{d-1}).
\\end{aligned}
 \\tag{67}
-->
![公式](../../assets/math/37f002bf86e2a4bc307bd08c.svg)

初始额度中的 <code class="math">T_{d-1}</code> 项用于支付之后动用初始额度时引发的子层更新。令 <code class="math">\mathcal H_d=J_d+T_d/k</code>，即可闭合递推：

<!-- math-display: 2302ac219f23fd9586a6a55e
\\mathcal H_d\\leq
 O(W+P_{\\mathrm b}(G_{d-1}+1))+O(P_{\\mathrm b})\\mathcal H_{d-1}.
 \\tag{68}
-->
![公式](../../assets/math/2302ac219f23fd9586a6a55e.svg)

因此所有工作量与出现项因子均有界于
<code class="math">k^{O(1)}W(cP_{\mathrm b})^{O(d)}</code>。
较小实际规模实例使用显式的多项式时间最小比率环基底求解器，并对多字长算术计费。关于 <code class="math">k</code> 的多项式基底因子在此包络中只出现一次。

局部距离比较与式 (56) 给出单层逆向质量 <code class="math">\Gamma^{O(K_{\mathrm r})}</code>，另有关于 <code class="math">U</code> 的固定多项式因子。跨 <code class="math">d</code> 层后，完整原始逆向质量的界为

<!-- math-display: fe86273bf4e6ac0dca0da620
k^{O(1)}\\exp O(dK_{\\mathrm r}\\log\\Gamma+d\\log U).
 \\tag{69}
-->
![公式](../../assets/math/fe86273bf4e6ac0dca0da620.svg)

同树局部环与修正环流的提取按其实际展开工作量计入 <code class="math">W</code>。原始证书必须覆盖它们的字面遍历。式 (69) 包含所有递归层。

### 11.5 不发生递归精度增长的有限位森林

这里所需的有限位树接口如下：给定一个连通的 <code class="math">n'</code> 顶点、<code class="math">m'</code> 边图，其原始边长均在 <code class="math">[1,2^b)</code> 内，返回一棵由原图边构成的生成树 <code class="math">T</code>，满足

<!-- math-display: 0cabacbdb870530f5b44fb96
\\sum_{e=uv}\\frac{d_T(u,v)}{\\ell_e}\\leq
       m'\\mathrm{poly}(b+\\log(n'+m'+4))
 \\tag{70}
-->
![公式](../../assets/math/0cabacbdb870530f5b44fb96.svg)

且位工作量为 <code class="math">(n'+m')\mathrm{poly}(b+\log(n'+m'+4))</code>。
[有限位花瓣树适配器](../../manuscripts/graph-backends/an-weighted-finite-bit-adapter.md)第 1–7 节使用 [AN] 的加权花瓣几何，证明了这一实现。它使用共同二进制网格上的精确加权门户、每个花瓣内固定的静态有向度量、精确中点检验，以及分别维护的工作路径与继承目标路径。它不使用短边收缩，并以关于位长宽度的多项式因子作为代价。输出原始边生成树前，会删除临时门户坐标。[稳定森林集成证明](../../manuscripts/graph-backends/polynomial-bit-stretch-stable-forest-integration.md)第 2–7 节给出了加权调用及下述无环位长界。

对加权森林调用，使用出现计数 <code class="math">N_e</code> 及整数重要度 <code class="math">w_e=N_e\ell_e</code>。若一个连通分量有 <code class="math">a</code> 条边且总权重 <code class="math">W_0&gt;0</code>，则以 <code class="math">v_e=aw_e+W_0</code> 平滑，并采用副本数 <code class="math">\lceil av_e/\sum_fv_f\rceil</code>，总数至多为 <code class="math">2a</code>。若 <code class="math">W_0=0</code>，则采用均匀正权重。倒数排序使用二进制代理 <code class="math">2^{-\lceil\log\ell_e\rceil}</code>。所有这些操作均使用整数算术或同一个二的幂网格。

关键的是，一个输出的固定伸展标签有如下精确表示：

<!-- math-display: f1323b69528c932cde46c50c
\\ell_e\\widetilde{\\mathrm{str}}(e)
       =2\\sum_{j=0}^{O(\\log^2(n'+2))}(\\ell_e+P_j(e)),
 \\tag{71}
-->
![公式](../../assets/math/f1323b69528c932cde46c50c.svg)

其中每个 <code class="math">P_j(e)</code> 是一条简单原始树路径，或两条根路径之和。若 <code class="math">L_{\max}</code> 为整个实例生命周期内父边长的上界，则标签至多为 <code class="math">O((n'+1)\log^2(n'+2)L_{\max})</code>。其大小与式 (70) 中的平均伸展，或加权森林调用次数无关。

冻结的修正梯度同样有界于 <code class="math">(2n'+1)G_{\max}</code>，此处使用生命周期而非当前梯度的界。若实际记录的规模守卫为 <code class="math">n'+m'\leq2^{c_{\rm size}U}</code>，则每个距离层只增加 <code class="math">O(U)</code> 位，另加端口缩放位。预先声明

<!-- math-display: 358c820e7f2e2544a9222366
b_*=c_b U(d_*+1),
 \\tag{72}
-->
![公式](../../assets/math/358c820e7f2e2544a9222366.svg)

其中固定常数 <code class="math">c_b</code> 足以覆盖根价格及规模常数。该标签界在代入多项式位数树成本之前就已证明。因此式 (70) 的输入精度为 <code class="math">\mathrm{poly}(U)</code>，其成本只带来固定多项式因子。

完整的初始未来路径出现项库存的规模和总边数为 <code class="math">m'k^{O(1)}\mathrm{poly}(U)</code>，最大路径长度至多为 <code class="math">k^{O(1)}\mathrm{poly}(U)2^{b_*}</code>。因此，未来路径加权二分所需森林数为
<code class="math">O(\log(m'+2)+\log(k+2)+b_*+\log(U+2))=\mathrm{poly}(U)</code>。
该数量已包括长度宽高因子。临时谱、矩或门户算术按局部工作计费，不会输出为存储数值标签。

<a id="116-explicit-raw-backend-realization-contract"></a>

### 11.6 确定性环搜索实现

下一个定理实现前述各接口。尤其是原始长度比较、出现项维护和有限位树构造，均由链接的证明给出，而非额外假定的未证基本工具。

**定理 11.6（实现与依赖闭合）。** 使用第 2.2 节的确定性来源结果、第 11.1 节的实际规模约定，以及参数族 (73)。对于所有充分大的主规模，取固定且足够大的上限常数，即可得到一个有限确定性动态算法，在所声明的多项式主历史上实现第 8.1 节接口。它涵盖不连通图、原生平行边记录、真正的正长度自环，以及根据已稳定历史自适应选择的表示变更。算法返回带符号的物理边与森林路径片段；其梯度精确，且长度证书覆盖所有抵消前的物理遍历。

在平衡参数之前，出现项、初始化与更新界为式 (67)–(68)，逆向原始质量为式 (69)。因此工作量和记录数量界于 <code class="math">k^{O(1)}W(cP_{\mathrm b})^{O(d_*)}</code>；独立的质量界包含 <code class="math">\exp O(d_*K_{\mathrm r}\log\Gamma+d_*\log U)</code>。同一工作包络也支付查询、其暴露日志和显式答案片段。存储长度与梯度满足式 (71)–(72)。所有多项式次数及隐含常数均固定，不随增长参数改变。

中间构造允许整数 <code class="math">1\leq p,r,K_{\mathrm r}\leq U</code> 且 <code class="math">(p+r)h\leq U</code>。每次局部调用均使用其实际居民图、局部来源允许的范围或指定的直接基底情形，以及第 11.5 节的位长守卫。第 12 节会检查参数 (73) 的收缩、深度、长宽比和数值客户端不等式；整个证明不会将不合规的参数代入某个已发表的完整层次结构。

| 所需接口 | 提供该接口的构造 | 界与适用范围 |
|---|---|---|
| 字面原始环质量与精确梯度 | [原始提升](../../manuscripts/graph-backends/distance-oracle-raw-lift.md)，第 2–7 节；[归一化环比较](../../manuscripts/graph-backends/normalized-route-cycle-comparison.md)，第 1–6 节；[度数重置](../../manuscripts/graph-backends/distance-degree-reset-candidate.md)，第 2–5 节 | 非环根路径、同根基本环、保留边规范修正及环流恒等式给出式 (56)、(65) 以及完整乘积 (69)。最终局部提取损失只支付一次。 |
| 按时间顺序的初始化、修复和所选边递归 | [静态基底修复](../../manuscripts/graph-backends/amortized-static-base-distance-sparsifier.md)，第 3–7 节；[投影路由组合](../../manuscripts/graph-backends/normalized-projected-route-composition.md)，第 1–5 节；[路由通知](../../manuscripts/graph-backends/demand-route-notifications-and-local-work.md)，第 2–6、8 节 | 每个前缀（包括尝试中的级联）均满足式 (59)–(63)。初始额度可以延后使用，并由式 (67) 的子层更新项支付。完整路由及逆向列表维护计入 <code class="math">W</code>；只有所选原生边的变化计入 <code class="math">P_{\mathrm b}</code>。 |
| 真正的出现项森林、非活跃记录与退役 | [度数重置与扁平池](../../manuscripts/graph-backends/distance-degree-reset-candidate.md)，第 2–7 节；[生命周期与自环构造](../../manuscripts/graph-backends/numerical-backend-lifetime-and-loops.md)，第 1–5 节；[不连通分量包装器](../../manuscripts/graph-backends/disconnected-component-backend-wrapper.md)，第 2、5–8 节 | 稳定纤维属于实际生命周期；非活跃与孤立副本计入式 (67)。在带符号增量与绝对移动量完成转移之前，旧映射始终有效。查询及其消耗构成一个原子事务。正长度自环具有单独的精确比率服务。 |
| 有限位原始边树及加性标签增长 | [有限位花瓣树适配器](../../manuscripts/graph-backends/an-weighted-finite-bit-adapter.md)，第 1–7 节；[稳定森林集成](../../manuscripts/graph-backends/polynomial-bit-stretch-stable-forest-integration.md)，第 2–7 节；[确定性路径并列规则](../../manuscripts/graph-backends/deterministic-symbolic-shortest-path-ties.md)，第 1–6 节 | 式 (70) 在长宽比位数的多项式工作量内成立。整数平滑和共同二进制倒数网格避免不相关分母。精确标签公式 (71) 保证每层只增加 <code class="math">O(U)</code> 位，之后才代入树构造成本。 |
| 统一有限路由器、静态提供器与共同上限 | [顺序路由器](../../manuscripts/graph-backends/sequential-expander-routing-tradeoff.md)，第 2–8 节；[线性参数静态提供器](../../manuscripts/graph-backends/linear-parameter-static-sparsifier.md)，第 2–7 节；[统一来源接口](../../manuscripts/graph-backends/uniform-source-interfaces-for-4-5.md)，第 1–8 节；[定量匹配组合](../../manuscripts/graph-backends/quantitative-matching-4-5-composition.md)，第 3–7 节 | 显式提供器满足 <code class="math">\log\Gamma=O((p+r)h)</code> 及 <code class="math">\log F=O(U/p+U/r+r^2h+(p+r)h)</code>。实际规模基底情形、临时位工作、两个见证查询量及所有递归质量因子均已计费。只需固定且可枚举的常数。 |

**证明。** 首先构造静态提供器，而不是假设其输出类型成立。[虚拟层次结构](../../manuscripts/graph-backends/virtual-expander-hierarchy-reconstruction.md)第 3–5.1 节给出字面的子图包含关系、修正后的覆盖保证，以及经单个根嵌入展平的完整路径族。精确短路径匹配、真实割适配器、阈值至多为八分之一的熵证明、能量割玩家和叶电路共同提供其局部过程。[完整路由归纳](../../manuscripts/graph-backends/conditional-full-router-from-shufflers.md)第 3–8 节随后以精确目的地路由真实与 dummy token。该证明的一般输入假设均由该层次结构及叶电路满足；顺序路由器第 6 节明确完成了这一实例化。因此，标题中包含“conditional”并不意味着本文路由器仍需额外假设。

静态提供器在两个加权见证方向均使用该路由器，并在两次调用中都按宿主图规模乘以负载计费。确定性加权路径选择和对当前图的删环操作，提供所选支撑与归一化路径。由此证明命题 10.1，并区分结构因子 <code class="math">\Gamma</code> 与构造因子 <code class="math">F</code>。较小的平衡割调用会将 <code class="math">r</code> 截断到其实际合法范围。证明从未调用 <code class="math">K_{\mathrm r}=t^2</code> 处的旧完整动态稀疏器定理。

然后，对每个实际稳定核心应用静态基底调度。前缀势函数证明在稳定之前界定全部反馈割；去重后的所选边 ID 决定度数与子层发布。路由所有者递推式给出式 (62)，并涵盖原先为空的路由的端点。显式正向与逆向列表枚举每条改变的修正路由；所选边保留其自身中心来源。因此，这部分局部工作由式 (63) 计费，不会把每次修正变化都转化为递归输入编辑。延后使用初始额度属于真实的时间顺序收费，因此式 (67) 同时保留 <code class="math">J_{d-1}</code> 和 <code class="math">T_{d-1}</code>。

插入稳定的三度纤维与扁平根池。收缩被指派的池树即可恢复子森林，证明扩展对象仍然是森林。子森林变化只改动一条连接记录或占用一个有计数的池槽；父割则另外施加于所有后代副本。池增长采用基于实际计数的倍增，其替换与退役工作按几何级数计费。生命周期构造会在旧映射有效时转移带符号流量及绝对移动量，保留非活跃副本，并将刷新排队至返回答案被消耗或丢弃之后。上述规则在每个完整事务上，以及在已稳定操作的自适应历史下，均保持数值恒等式。

对于质量界，原始提升证明界定每次字面提升遍历。同根边使用其简单基本环；非自环使用两条根路径。保留边在零连接收缩之前先施加冻结势能修正。因此梯度在完整环流上望远镜消去，原始长度按所需方向受界定。归一化路由恒等式保留其修正备选项，度数重置则通过式 (65) 保持分子并支配长度。将所有层的损失相乘即得式 (69)；终端提取损失不会在每层重复计入。

对于算术，花瓣适配器以关于 <code class="math">b+\log(n'+m'+4)</code> 的多项式成本证明加权原始边树接口。其门户使用单一二进制网格，临时坐标不会变成数值标签。固定标签表达式 (71) 只依赖简单原始树路径，因此其大小独立于平均伸展或加权调用次数。这在计入多项式树工作量之前先确立了式 (72)。未来路径加权二分包括长宽比项及全部初始单边。确定性符号并列规则、整数副本和精确条件期望均不引入随机种子或未计费实数运算。

其余操作都是有限扫描、字典操作、显式路径操作，或在声明适用范围内调用列名的确定性来源工具。因此递推常数一致固定。式 (67)–(69) 与无环位长归纳给出所述工作量、记录量和质量界。第 12 节将它们同时代入，以获得一组有利的固定上限参数；该节对有限个例外规模使用独立检验过的经典工作器处理。∎

**已发表结果的边界与实现覆盖范围。** Kyng、Meierhans、Probst Gutenberg 与 Sulser 的论文《[A Simpler and Faster Min-Cost Flow Solver via Min-Ratio Cycles from Distance Oracles](https://arxiv.org/html/2609.23852v1)》，arXiv:2609.23852v1，定理 4.4–4.5，给出了其自身的确定性 <code class="math">\exp(O(\log^{20/21}m\log\log m)</code> 层次结构。该参数选择并不是本文定理的证明。其零流插入操作在此由已证明的生命周期转移补充；原始提升证明也提供了所需的物理长度上界，以替代其引理 5.4 中印出的方向。新的路由、修复与有限位构造提供改进参数。这些都是 10 月 24 日构造链中已经包含的数学算法与证明。端到端快速代码仍属于后续实现工作；现有参考求解器具有独立的多项式界。[来源图](sources.md)标明所有引入论文，[验证说明](verify/README.md)记录较窄范围的可执行证据。

## 12. 参数平衡与单一有限程序

### 12.1 取整参数与全部计费项

对充分大的 <code class="math">U</code>，计算

<!-- math-display: 70cb91dea460f92a1d82b046
\\begin{gathered}
 h=\\lceil\\log(U+4)\\rceil,\\qquad
 t=\\left\\lfloor(U/h)^{1/5}\\right\\rfloor,\\qquad
 p=r=t,\\\\
 K_{\\mathrm r}=t^2,\\qquad x=\\lceil U/t\\rceil,\\qquad
 k=2^x,\\qquad d_*=\\lceil8U/x\\rceil+10.
\\end{gathered}
 \\tag{73}
-->
![公式](../../assets/math/70cb91dea460f92a1d82b046.svg)

只需比较整数根。检验 <code class="math">t\geq2</code>、<code class="math">(p+r)h\leq U</code> 及 <code class="math">K_{\mathrm r}\leq U</code>。有限个例外规模由下文经典工作器处理。我们有

<!-- math-display: a676bc3f1ba1f45f42153a68
t^5h\\leq U,\\qquad
 x=\\Theta(U^{4/5}h^{1/5}),\\qquad d_*=O(t).
 \\tag{74}
-->
![公式](../../assets/math/a676bc3f1ba1f45f42153a68.svg)

在运行前选取二进制上限常数。一种方便的无环参数族由整数 <code class="math">c_C</code> 索引，令
<code class="math">c_\phi=\max\{1,\lfloor c_C^{1/4}\rfloor\}</code>、
<code class="math">c_\Lambda=\max\{1,\lfloor c_C^{1/2}\rfloor\}</code>、
<code class="math">c_\Gamma=\max\{1,\lfloor c_C^{3/4}\rfloor\}</code>。
取扩张率上限 <code class="math">2^{-c_\phi rh}</code>、路由上限 <code class="math">2^{c_\Lambda(p+r)h}</code>、静态上限 <code class="math">2^{c_\Gamma(p+r)h}</code>，以及对数为
<code class="math">O(c_C(\lceil U/K_{\mathrm r}\rceil+(p+r)h+h))</code>
的端口／记录上限。其常数按上述顺序逐级支配。固定一个足够大的 <code class="math">c_C</code> 后，再用一个独立的固定常数 <code class="math">c_E</code> 统摄所有工作量与逆向质量。

由式 (48)，

<!-- math-display: 87c5366164568d8ace169da9
\\log\\Gamma=O(th),\\qquad
 \\log F=O(U/t+t^2h+th).
 \\tag{75}
-->
![公式](../../assets/math/87c5366164568d8ace169da9.svg)

完整的工作／质量包络至多为下式的常数倍：

<!-- math-display: 80fe99d45a1f4c4ff77284b9
x+\\log F+K_{\\mathrm r}\\log\\Gamma+
       \\frac Ux\\left(\\frac{U}{K_{\\mathrm r}}
                  +\\log\\Gamma+K_{\\mathrm r}\\log\\Gamma+h\\right).
 \\tag{76}
-->
![公式](../../assets/math/80fe99d45a1f4c4ff77284b9.svg)

下表逐项支付这些成本。

| 费用 | 参数 (73) 下的界 |
|---|---|
| 递归规模缩减与直接基底 | <code class="math">x=O(U/t)</code> |
| 静态构造 | <code class="math">\log F=O(U/t+t^2h)</code> |
| 局部修复工作 | <code class="math">K_{\mathrm r}\log\Gamma=O(t^3h)</code> |
| 传播记录、工作量与初始额度 | <code class="math">d_*\log P_{\mathrm b}=O(U/t+t^2h+th)</code> |
| 完整递归逆向质量 | <code class="math">d_*K_{\mathrm r}\log\Gamma=O(t^4h)</code> |
| 存储标签算术 | <code class="math">b_*=O(Ut)</code>，按多项式计费 |

由于 <code class="math">t^5h\leq U</code>，所有对数开销均为 <code class="math">O(U/t)</code>。此外，<code class="math">\log P_{\mathrm b}/x=O(1/t+t^2h/U)=o(1)</code>，因此最终有 <code class="math">P_{\mathrm b}/k\leq k^{-1/2}</code>；<code class="math">d_*</code> 留有固定余量。对非基底图，<code class="math">\log m'\geq x-O(1)</code>，且 <code class="math">b_*/x^2=O(t^3/U)=o(1)</code>，满足存储标签长宽比守卫。较小实例使用直接计费的基底求解器。

由定理 11.6，取一组固定且足够大的常数对 <code class="math">(c_C,c_E)</code>，即可得到可计算的完整上限

<!-- math-display: b1bbf358860e881023720016
E_*=2^{c_E x},\\qquad
 \\log E_*=O(U^{4/5}(\\log(U+4))^{1/5}).
 \\tag{77}
-->
![公式](../../assets/math/b1bbf358860e881023720016.svg)

它覆盖每条实际记录的初始化、摊还更新、查询、完整日志、出现项、答案片段及逆向原始质量。若根答案至多有 <code class="math">E_*</code> 个片段，且其森林最多含 <code class="math">ME_*</code> 条记录，则原始分子与分母的绝对值至多为 <code class="math">ME_*^2</code> 乘以相应根标签上界。对有利的固定参数组，其位数为 <code class="math">O(U)</code>。维护的路径聚合结构会直接提供每条返回森林路径的长度及带符号梯度和，无需展开路径来计算证书。实际原始分母只可能改善其相对于继承上界证书的比率。

### 12.2 一致常数与独立验收

为得到一个无需关于“常数充分大”数值建议的单一程序，按固定可计算顺序枚举正整数对 <code class="math">(c_C,c_E)</code>。调度器为一个经典确定性精确工作器保留固定正指令份额，并为每个固定参数组分配固定正份额，例如采用二进制尺码调度。参数组生成与指令模拟均计费。任何工作器都不会预先分配其主规模上界。

每个工作器在使用结果前都检查度数、规模、历史、前缀递归、深度、标签及工作量上限。违反上限则暂停；路径过长时绝不截断。接受任何候选之前，先验证其边列表，并在共享紧凑图上运行一次独立的确定性增广路搜索 [Gabow]。不存在增广路即认证最大基数。验证与候选物化成本均计入该工作器的指令份额。

该安排始终终止并返回正确答案：经典工作器获得正份额并提供一个答案；而没有通过增广路判据的候选绝不会被接受。定理 11.6 与第 12.1 节不等式证明，对每个充分大的输入，总有一个固定参数组是有利的。为说明守卫不会引入新假设，考虑其首次声称失败的时刻。此前所有事务均满足不变量；按时间顺序的前缀界也覆盖正在尝试的批次。因此工作量、规模、标签和深度均符合该参数组的上限，与首次失败矛盾。调度只带来常数因子，有限字长模拟开销为关于 <code class="math">s</code> 的多项式，而其经验证的完成时间由式 (31) 界定。有限个输入规模的例外可吸收到一致常数中。仅靠输出验证并不能证明存在有利参数组或运行时间界。

经典工作器还将终止前的总指令数及分配记录数界定为紧凑实例规模的固定多项式（另加调度开销）。因此地址和活动调度索引只需 <code class="math">O(s)</code> 位。这也证明了对持久内存与参数组合使用收费字长模拟是合理的。

### 12.3 定理 1.1 的完成

将 <code class="math">U=2s</code> 及式 (77) 代入式 (31)。由于清理后次三次构造满足 <code class="math">S=\Theta(e+1)</code>，关于 <code class="math">s</code> 的固定多项式因子均可吸收到式 (1) 的指数项中。清理与可选边核构造耗时 <code class="math">O(n+m)</code>；候选检查只使用共享的、保持精确性的紧凑图。最后仅将已接受匹配的边标识映射一次。引理 3.1 与 3.2 给出原始输入上的精确性，以及第二项中使用边核后的改进形式。定理 1.1 得证；证明使用了定理 11.6 的实例化构造及第 2.2 节所列来源结果。∎

## 参考文献

**[OAI]** OpenAI。《*Almost-Linear-Time Maximum-Cardinality Matching in General Graphs*》。稿件日期：2026 年 9 月 24 日，第 3–5 节。[官方稿件目录](https://github.com/openai/math/tree/main/preprints/Almost-Linear-Time-Maximum-Cardinality-Matching-in-Sparse-General-Graphs-September-24-2026)。

**[AHLT]** Stephen Alstrup、Jacob Holm、Kristian de Lichtenberg 与 Mikkel Thorup。《*Maintaining Information in Fully-Dynamic Trees with Top Trees*》。2003 年第 2 版；发表于 *ACM Transactions on Algorithms*，2005。[arXiv:cs/0310065v2](https://arxiv.org/abs/cs/0310065v2)。

**[CHHIP]** Shiri Chechik、Thomas Dueholm Hansen、Giuseppe F. Italiano、Veronika Loitzenbauer 与 Nikos Parotsidis。《*Faster Algorithms for Computing Maximal 2-Connected Subgraphs in Sparse Directed Graphs*》。2017 年，第 3.1 节、引理 3.5。[arXiv:1705.10709v1](https://arxiv.org/abs/1705.10709v1)。

**[KMPG]** Rasmus Kyng、Simon Meierhans 与 Maximilian Probst Gutenberg。《*A Dynamic Shortest Paths Toolbox: Low-Congestion Vertex Sparsifiers and their Applications*》。2023 年，定理 3.2、3.8，引理 3.5 及附录 A.1。[arXiv:2311.06402v1](https://arxiv.org/abs/2311.06402v1)。

**[CKL]** Li Chen、Rasmus Kyng、Yang P. Liu、Richard Peng、Maximilian Probst Gutenberg 与 Sushant Sachdeva。《*Maximum Flow and Minimum-Cost Flow in Almost-Linear Time*》。2022 年；确定性单森林构造与固定标签见引理 6.5–6.6 及附录 B.3。[arXiv:2203.00671v1](https://arxiv.org/abs/2203.00671v1)。

**[AN]** Ittai Abraham 与 Ofer Neiman。《*Using Petal-Decompositions to Build a Low Stretch Spanning Tree*》。*SIAM Journal on Computing* 48(2)，2019。[DOI:10.1137/17M1115575](https://doi.org/10.1137/17M1115575)。

**[CGLNPS]** Julia Chuzhoy、Yu Gao、Jason Li、Danupon Nanongkai、Richard Peng 与 Thatchaphol Saranurak。《*A Deterministic Algorithm for Balanced Cut with Applications to Dynamic Connectivity, Flows, and Beyond*》。2020 年第 2 版，定理 2.6、5.1。[arXiv:1910.08025v2](https://arxiv.org/abs/1910.08025v2)。

**[CS21]** Julia Chuzhoy 与 Thatchaphol Saranurak。《*Deterministic Algorithms for Decremental Shortest Paths via Layered Core Decomposition*》。2020 年预印本；确定性扩张子剪枝接口见定理 3.7。[arXiv:2009.08479v1](https://arxiv.org/abs/2009.08479v1)。

**[CS]** Yi-Jun Chang 与 Thatchaphol Saranurak。《*Deterministic Distributed Expander Decomposition and Routing with Applications in Distributed Derandomization*》。2020 年，第 4.1–4.3 节及层次结构几何。[arXiv:2007.14898v1](https://arxiv.org/abs/2007.14898v1)。

**[CHS]** Yi-Jun Chang、Shang-En Huang 与 Hsin-Hao Su。《*Deterministic Expander Routing: Faster and More Versatile*》。2024 年，层次结构与洗牌器架构。[arXiv:2405.03908v1](https://arxiv.org/abs/2405.03908v1)。

**[CKLMPG]** Li Chen、Rasmus Kyng、Yang P. Liu、Simon Meierhans 与 Maximilian Probst Gutenberg。《*Almost-Linear Time Algorithms for Incremental Graphs: Cycle Detection, SCCs, <code class="math">s</code>-<code class="math">t</code> Shortest Path, and Minimum-Cost Flow*》。2023 年，第 7.2 节，触及集调度模板。[arXiv:2311.18295v1](https://arxiv.org/abs/2311.18295v1)。

**[KMPS]** Rasmus Kyng、Simon Meierhans、Maximilian Probst Gutenberg 与 Aurelio Sulser。《*A Simpler and Faster Min-Cost Flow Solver via Min-Ratio Cycles from Distance Oracles*》。2026 年，局部到环距离比较。[arXiv:2609.23852v1](https://arxiv.org/abs/2609.23852v1)。

**[Gabow]** Harold N. Gabow。《*The Weighted Matching Approach to Maximum Cardinality Matching*》。2017 年；首次搜索的正确性与实现见第 3、5 节。[arXiv:1703.03998v1](https://arxiv.org/abs/1703.03998v1)。

---

**署名：** icekylinx 与 GPT-6 Astra Pro。

证明使用上述具名外部接口。有限检验与证明助手验证彼此独立。
