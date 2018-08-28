# OneM2M

## Common Service Functions (CSF)

### Data Management and Repository

目的：讓 AE 可以互相交換資料

負責提供資料儲存，還有協調的 function。而且還包括收集資料 (為了整合大量的資料)，而且把資料轉換成特別的格式，然後儲存它 (為了分析)。這些資料可以是剛從 M2M Device 獲得的 raw data，也可以是 M2M entities 運算或整合過的資料。

NOTE：收集巨大的資料最有名的是 Big Data Repository，並不會在這個文件中。

## Registration

讓 AE 或 CSE 發出註冊的請求給 Registrar CSE，為了可以使用 Registrar CSE 的服務。

| Originator (Registree) | Receiver (Registrar) | Registration Procedure |
| ---------------------- | -------------------- | ---------------------- |
| ADN-AE                 | MN-CSE, IN-CSE       | AE                     |
| ASN-AE                 | ASN-CSE              | AE                     |
| MN-AE                  | MN-CSE               | AE                     |
| IN-AE                  | IN-CSE               | AE                     |
| ASN-CSE                | MN-CSE, IN-CSE       | CSE                    |
| MN-CSE                 | MN-CSE, IN-CSE       | CSE                    |



## Resource 欄位

### Resource Type

| 0     | mixed                                |
| ----- | ------------------------------------ |
| 1     | accessControlPolicy                  |
| 2     | AE                                   |
| 3     | container                            |
| 4     | contentInstance                      |
| 5     | CSEBase                              |
| 6     | delivery                             |
| 7     | eventConfig                          |
| 8     | execInstance                         |
| 9     | group                                |
| 10    | locationPolicy                       |
| 11    | m2mServiceSubscription               |
| 12    | mgmtCmd                              |
| 13    | mgmtObj                              |
| 14    | node                                 |
| 15    | pollingChannel                       |
| 16    | remoteCSE                            |
| 17    | request                              |
| 18    | schedule                             |
| 19    | serviceSubscribedAppRule             |
| 20    | serviceSubscribedNode                |
| 21    | statsCollect                         |
| 22    | statsConfig                          |
| 23    | subscription                         |
| 24    | semanticDescriptor                   |
| 25    | notificationTargetMgmtPolicyRef      |
| 26    | notificationTargetPolicy             |
| 27    | policyDeletionRules                  |
| 28    | flexContainer                        |
| 29    | timeSeries                           |
| 30    | timeSeriesInstance                   |
| 31    | role                                 |
| 32    | token                                |
| 33    | trafficPattern                       |
| 34    | dynamicAuthorizationConsultation     |
| 10001 | accessControlPolicyAnnc              |
| 10002 | AEAnnc                               |
| 10003 | containerAnnc                        |
| 10004 | contentInstanceAnnc                  |
| 10009 | groupAnnc                            |
| 10010 | locationPolicyAnnc                   |
| 10013 | mgmtObjAnnc                          |
| 10014 | nodeAnnc                             |
| 10016 | remoteCSEAnnc                        |
| 10018 | scheduleAnnc                         |
| 10024 | semanticDescriptorAnnc               |
| 10028 | flexContainerAnnc                    |
| 10029 | timeSeriesAnnc                       |
| 10030 | timeSeriesInstanceAnnc               |
| 10033 | trafficPatternAnnc                   |
| 10034 | dynamicAuthorizationConsultationAnnc |
| 20001 | oldest                               |
| 20002 | latest                               |



### AccessControlPolicy

#### privileges and selfPrivileges

- *privileges*：一組 access control rules，套用所有連到這個 <*accessControlPolicy>* 的 resources 上，透過 *accessControlPolicyID*。
- *selfPrivileges*：會把規則套用在自己的 <*accessControlPolicy>* resource 上，還有連到這個 acp 的 resource 上。

*privileges* 跟 *selfPrivileges* 會由四個 tuples 所組成：

- *accessControlOriginators* (acor)：who
- *accessControlContexts* (acco)：where
- *accessControlOperations* (acop)：what
- *accessControlObjectDetails* (acod)：

如果 *privileges* 沒有包含上面四個 tuple，這代表是一組空的 access control rules。

*selfPrivileges* 最少要有一個 tuple。

CSE access grating mechanism 定義在 TS-0003 7.1 Access Control Mechanism

##### accessControlOriginators

在 access-control-rule-tuple 裡是一個強制性的參數，代表一組的 Originators，會被套用 access control rule，這組的 Originator 是一個 list，會由 CSE 來檢查這個權限。

有四個參數的 type：

- domain：SP domain 或 SP sub-domain
- originatorID：CSE-ID、AE-ID 或 <*group*> resource 裡面包含 AE 或 CSE 的 resource-ID
- all：允許任何 Originator 存取資源
- Role-ID

Hosting CSE 會檢查 originator 的請求是否符合 <*group*> resource 裡的其中一個 member (欄位是 memberID)，如果 <*group*> resource 不存在或是 retrieve 不到的話，那 request 就會被拒絕。

##### accessControlContexts

選擇性參數

- *accessControlTimeWindow*
- *accessControlLocationRegion*
- *accessControlIpAddresses*

##### accessControlOperations

強制性參數

- RETRIEVE
- CREATE
- UPDATE
- DELETE
- DISCOVER
- NOTIFY

##### accessControlObjectDetails

選擇性參數

- resourceType：access control rule 會套用在哪個 resource type 上
- specialization：如果 *resourceType* 是 *mgmtObj* 或 *flexContainer* 的話，specialization 這個欄位會分別定義在 *mgmtDefinition* 或 *containerDefinition* 的屬性裡面。
- childResourceType：child resource types 或 specialization identifier 的 list

如果有 *accessControlObjectDetails* 的話，應該要有 *childResourceType*，會套用 access control rule 在這個 list 上。

resourceType 和 specialization 這兩個欄位是選擇性的，只要有其中一個存在的話，在檢查 *childResourceType* 就會更近一步的檢查，如果沒有提供就不會執行檢查。

##### accessControlAuthenticationFlag

選擇性欄位，預設是 FALSE

- TRUE：只有在 Originator 是被 Hosting CSE 授權的時候，才適用 access control rule
- FALSE：不論 Originator 有沒有被 Hosting CSE 授權，都適用 access control rule



```json
{
    "pv": {
        "acr": [
            {
                "acor": [
                    "guest_groupA",
                    "guest_groupB"
                ],
                "acop": 51
            },
            {
                "acor": [
                    "S20170705065326333aZtE"
                ],
                "acop": 63
            }
        ]
    },
    "pvs": {
        "acr": [
            {
                "acor": [
                    "S20170705065326333aZtE"
                ],
                "acop": 63
            }
        ]
    }
}
```



### CSEBase 

CSEBase resource 應該是在 CSE 中所有 resource 的源頭 root。

#### CSE-ID

在 M2M-SP Domain 下的 hosting CSE ID

#### cseType

- 強制在 IN-CSE 要有這個欄位
- ASN-CSE 或 MN-CSE 則受 SP 的設置影響

#### pointOfAccess

可以讓 remote CSE 使用 point of access 所提供的 physical addresses 的列表，來連到自己的 CSE。

### remoteCSE

簡寫：csr

remote CSE 代表 Registree CSE，可以向 Registrar CSE 註冊。<*remoteCSE*> 的資源會在 Registrar CSE <*CSEBase*> 資源的底下。

相似的，remote CSE 也代表 Registrar CSE，<*remoteCSE*> 的資源會在 Registree CSE <*CSEBase*> 資源的底下。

都是 <*remoteCSE*> 的資源會在 <*CSEBase*> 底下。不過註冊的資料會分別在兩個 CSE 底下。

舉例，當 CSE1 (Registree CSE) 向 CSE2 (Registrar CSE) 註冊的時候，會有兩個 resources 被建立：一個在 CSE1 底下的 <*CSEBase1*>/<*remoteCSE2*> 跟 CSE2 底下的 <*CSEBase2*>/<*remoteCSE1*>

但這不代表他們互相註冊，<*CSEBase1*>/<*remoteCSE2*> 這個並不代表 CSE2 向 CSE1 註冊。

### remoteCSE

#### requestReachability

如果這個 <*remoteCSE*> 可以從其他的 AE/CSE(s) 接收到 request 的話，這個值要設成 TRUE，其餘為 FALSE。

如果設為 FALSE 並不代表永遠無法到達這個 CSE，像是使用 NAT，只有在相同 NAT 底下才能通訊。

### container

用來裝資料實例，而且被用來跟其他的 entity 分享訊息，和可能用來追蹤資料，container 資源裡不會連帶 content。

- maxNrOfInstances
- maxByteSize
- maxInstanceAge：單位 秒
- currentNrOfInstances
- currentByteSize

#### disableRetrieval

如果設為 TRUE，對 child <*contentInstance*> 的 RETRIEVE/DELETE/UPDATE 這些操作會被拒絕。

如果將這個值從 TRUE 更新為 FALSE，會立即將所有存在的 <*contentInstance*> 刪除。

如果為 FALSE 就允許所有操作。

Mobius 設定這個值為 Null。

### contentInstance

代表在 <*container*> 資源裡的資料實例 data instance。可以加密 *contentInstance* 的 content 。

不像其他 resources，一旦建立了 <*contentInstance*> 就不能做任何修改，AE 能夠刪除 *contentInstance*，或是平台有自己的政策，像是在 <*container*> 裡的這些屬性 *maxByteSize*、*maxNrOfInstances*、*maxInstanceAge*，會保留 *contentInstance* 一段時間，如果同時使用多個 policy，會以套用最嚴格的那個。

<*contentInstance*> resource 的 access control policy 會繼承 <*container*> resources 的，它並沒有自己的 *accessControlPolicyIDs* 的欄位。

#### content (con)

這個 content 是不透明的資料

#### contentSize (cs)

Content 欄位的大小 byte。

#### contentInfo (cnf)

是由兩個元件所組成的 Internet Media Type (RFC 6838) 和 encoding type。

| contentInfo Value | Interpretation                                   | Note |
| ----------------- | ------------------------------------------------ | ---- |
| 0 | ESData has not been applied to the content data. | |
| 1 | ESData using JWE and/or JWS with Compact Serialization has been applied to the content data with no subsequent transfer encoding. See note 2. | See note 3 |
| 2 | ESData using JWE and/or JWS with JSON Serialization has been applied to the content data with no subsequent transfer encoding. | See note 4 |
| 3 | ESData using JWE and/or JWS with JSON Serialization has been applied to the content data and subsequent base64 encoding (see [9]) has been applied. | See note 4 |
| 4 | ESData using XML encryption and/or XML-Signature has been applied to the content data has been applied with no subsequent transfer encoding. | See note 5 |
| 5 | ESData using XML encryption and/or XML-Signature has been applied to the content data and subsequent base64 encoding (see [9]) has been applied. | See note 5 |



應該要使用這個屬性，AE 才知道這個內容。

### subscription

### group

代表一組相同或混合的 type，可以用來處理大量操作，透過 memberID 所代表的資源。<*group*> resource 包含一個屬性代表 group 的成員和 <*fanOutPoint*> virtual resource，能夠通用操作在這些成員的 resource 上。藉由組和分散的 <*semanticDescriptor*> resource，另一個 resource (<*semanticFanOutPoint*>) 透過 semantic discovery procedures 能夠使邏輯 tree 更加完整。

#### memberType (mt)

- same
- mixed

#### memberID

member resource IDs 的清單

#### consistencyStrategy

這個屬性說明，如果 *memberType* 的檢驗失敗的話，要如何處理 <*group*> resource，可能的參數：

- ABANDON_MEMBER：刪除不符的 member
- ABANDON_GROUP：刪除 group
- SET_MIXED：把 memberType 改成 mixed

預設是 ABANDON_MEMBER

#### memberTypeValidated

表示 group 的 member resource 的 resource type 是否有經過 Hosting CSE 的驗證。<*group*> resource 的 memberType 不能是 mixed，這樣這個屬性才會被設定。

### fanOutPoint

這個是 <*group*> resource 下的虛擬 resource，他會將每個 member 裡面的 resource 整合好回傳給 Originator

### timeSeries (ts)

是 Time Series Data instance 的 container，不會有相關的資料內容，用來 track、detect、report missing data。

#### periodicInterval (pei)

期望是一段 time，在兩個 Time Series Data instances 之間的週期性間隔。

#### missingDataDetect (mdd)

如果是週期性的資料，Receiver 應該能偵測 missing Time Series Data。

#### missingDataMaxNr (mdn)

在 *missingDataList* 裡面最大的數量，條件是有設定 *periodicInterval* 且 *missingDataDetect* 是 TRUE 的情況下。

### timeSeriesInstance (tsi)

- dataGenerationTime (dgt)
- sequenceNr (sqn)
- content (con)

## Object Identifier (OID)

http://www.oid-info.com/

- itu-t (0)
- Iso (1)
- joint-iso-itut-t (2)

### OID Based M2M Device Identifier

- M2M Device Indication ID
- Manufacturer ID
- Model ID
- Serial Number ID
- Expanded ID

台灣：2.16.886

