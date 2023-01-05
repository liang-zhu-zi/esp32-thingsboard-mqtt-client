Telemetry - Time-series data
===============================================

遥测(Telemetry)系统的目标, 就是获取时序数据(Time-series data), 进而对其进行监测、处理、分析等。

概念 Concept
-----------------

时序维度(Time-series axis, Time-series dimension) 
++++++++++++++++++++++++++++++++++++++++++++++++++++

被监测的目标。一个装置可以有多个 Time-series axis, 例如, 室内温度、湿度。


采样频率 ———— 采样周期
+++++++++++++++++++++++++

Time-series axis 的采集周期。


时间戳 Timestamp
+++++++++++++++++++++

采样的时刻, Unix timestamp格式, 精确到毫秒。

.. tip:: 
   * Timestamp 可以由本地装置产生，也可由服务器产生（接收的时刻）.

数据点 DataPoints
++++++++++++++++++++++

Time-series axis 每被采集一次，就获得一个 Data Point。

.. tip:: 
   * 一次上传, 可以包括不同 Time-series axis 的多个 DataPoint.
   * 每个 DataPoint 都会在 Server 上保存，直至超过期限。


.. tip:: 
   * Time-series data 对 Server 的网络带宽, CPU 处理能力和磁盘存储空间影响巨大。
   * 需要重点监控的数据，才以 Time-series data 的形式上传。
   * 根据实际需要，确定 Time-series axis 的采样频率，不可太短也不可太长。太短会消耗 Server 的带宽与处理能力, 并在Server形成大量冗余数据, 太长会造成 Server 无法针对 Time-series axis 做出实时响应。例如, 若烟感每10分钟采集一次到数据并上报, 会造成软大延时。
   * Time-series data 以 json 数据格式上传，故其 key 不应太长，会消耗带宽; 若 key 类似密文或数字，也会带来维护难度。


Working with telemetry data
--------------------------------

See `Working with telemetry data`__.

.. __: https://thingsboard.io/docs/user-guide/telemetry/


.. uml::

   title  Telemetry data

   participant "Device" as TBDev order 10
   participant "ThingsBoard Server"  as TBSrv order 20 
   box "Server-side Application" #LightBlue
   participant "Server-side Application" as TBApp  order 30
   participant "Web UI" as WebUI  order 40
   end box

   == Telemetry Upload ==

   TBDev  ->  TBSrv: Telemetry upload API (**Device API**)

   == Timeseries data Query ==

   TBSrv  <-  TBApp: Timeseries data keys API (**REST API**)
   TBSrv  <-  TBApp: Timeseries data values API (**REST API**)
   ... other **Timeseries data Query API** ...

   == or Timeseries data Subscription ==

   TBSrv  <-  WebUI: subscription commands (**Websocket API**)
   TBSrv  ->  WebUI: subscription updates (**Websocket API**)


Data points
-----------------


Improvement
-----------------

* 把 Alarm 机制由 Server 移到 Client/Deivce 端; 或者在 Client/Deivce 端加一个新的较弱 Alarm 机制; 或者只是为 Time-series data 加几个属性, 告诉 Client/Device 该如何更好的上传 Time-series data:
  
  * 最小值 Alram: 低于该值即刻upload
  * 最大值 Alarm: 高于该值即刻upload
  * 上一次 Upload 后的变化值 Delta: 差值大于该值即刻upload

  * 优势：

    * 减少数据冗余
    * 提升实时响应能力

  * 劣势：

    * 增加 Client/Deivce 端的复杂度
