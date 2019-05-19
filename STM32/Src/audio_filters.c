#include "audio_filters.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "arm_math.h"
#include "wm8731.h"
#include "settings.h"
#include "noise_reduction.h"

arm_fir_instance_f32    FIR_TX_Hilbert_I;
arm_fir_instance_f32    FIR_TX_Hilbert_Q;
arm_iir_lattice_instance_f32 IIR_LPF_I;
arm_iir_lattice_instance_f32 IIR_LPF_Q;
arm_iir_lattice_instance_f32 IIR_HPF_I;
arm_iir_lattice_instance_f32 IIR_HPF_Q;
arm_iir_lattice_instance_f32 IIR_Squelch_HPF;

//states
float32_t   Fir_Tx_Hilbert_State_I[FIR_TX_HILBERT_STATE_SIZE];
float32_t   Fir_Tx_Hilbert_State_Q[FIR_TX_HILBERT_STATE_SIZE];
float32_t		IIR_LPF_I_State[IIR_LPF_Taps_STATE_SIZE];
float32_t		IIR_LPF_Q_State[IIR_LPF_Taps_STATE_SIZE];
float32_t		IIR_HPF_State_I[IIR_HPF_Taps_STATE_SIZE];
float32_t		IIR_HPF_State_Q[IIR_HPF_Taps_STATE_SIZE];
float32_t		IIR_HPF_SQL_State[IIR_HPF_SQL_STATE_SIZE];

//with +/-45 degrees phase added, 48000 sampling frequency Fc=1.50kHz, BW=2.70kHz Kaiser, Beta = 3.650, Raised Cosine 0.910
const float32_t i_tx_coeffs[IQ_TX_HILBERT_TAPS] = { -0.000015911433738947,-0.000019959728028938,-0.000023891471599754,-0.000027119763124069,-0.000029087039705292,-0.000029431397715954,-0.000028148219961633,-0.000025702247123746,-0.000023048146401552,-0.000021532467002145,-0.000022675017711946,-0.000027858477628481,-0.000037984418906156,-0.000053173935947263,-0.000072594827015540,-0.000094480871554435,-0.000116372847662635,-0.000135561336982947,-0.000149658359810178,-0.000157181343994879,-0.000158011427902595,-0.000153597634188139,-0.000146821594676398,-0.000141508706977174,-0.000141657313079918,-0.000150538332814799,-0.000169872539761493,-0.000199303342326905,-0.000236339821701096,-0.000276850226155669,-0.000316055860965538,-0.000349836665758744,-0.000376046667777827,-0.000395483070998372,-0.000412180921252682,-0.000432823344456314,-0.000465251321301123,-0.000516291531622259,-0.000589344418113810,-0.000682329334357383,-0.000786618128311585,-0.000887471604155052,-0.000966224936321975,-0.001004084647738534,-0.000986971617519555,-0.000910465431454767,-0.000783674244635495,-0.000630853460997997,-0.000489869695624849,-0.000407142410379079,-0.000429421385599579,-0.000593545407736026,-0.000916011846980452,-0.001384598358663629,-0.001954277926711752,-0.002549184202610570,-0.003071435226054831,-0.003416332297185353,-0.003492031457300130,-0.003240513223388775,-0.002655839887826624,-0.001795531271501309,-0.000781549566761797,0.000211143419483594,0.000978377875720550,0.001320403087257693,0.001080515967568864,0.000182121345956079,-0.001343074868529297,-0.003342478753804935,-0.005550446581697207,-0.007618265957299014,-0.009164142003551838,-0.009837061742258062,-0.009385008993095094,-0.007715887072085315,-0.004939218878809190,-0.001378502462245723,0.002452076537338900,0.005907488065309155,0.008302997095508243,0.009025375745234239,0.007648260210940124,0.004033210841998318,-0.001601654938770521,-0.008660246045580289,-0.016203010335164171,-0.023038057426319945,-0.027860750884303231,-0.029425395934213904,-0.026726377512887117,-0.019162929429234910,-0.006662334542242190,0.010259113968032065,0.030510126124987710,0.052515877605683242,0.074378297568183149,0.094086304242110563,0.109749301278724468,0.119823145137080742,0.123298109645664233,0.119823145137071957,0.109749301278707773,0.094086304242087637,0.074378297568156199,0.052515877605654772,0.030510126124960246,0.010259113968007870,-0.006662334542261386,-0.019162929429248053,-0.026726377512893910,-0.029425395934214775,-0.027860750884299197,-0.023038057426312451,-0.016203010335154873,-0.008660246045570802,-0.001601654938762230,0.004033210842004407,0.007648260210943465,0.009025375745234765,0.008302997095506299,0.005907488065305405,0.002452076537334174,-0.001378502462250567,-0.004939218878813399,-0.007715887072088339,-0.009385008993096640,-0.009837061742258109,-0.009164142003550596,-0.007618265957296863,-0.005550446581694617,-0.003342478753802375,-0.001343074868527162,0.000182121345957515,0.001080515967569481,0.001320403087257519,0.000978377875719736,0.000211143419482372,-0.000781549566763161,-0.001795531271502565,-0.002655839887827574,-0.003240513223389301,-0.003492031457300196,-0.003416332297185003,-0.003071435226054171,-0.002549184202609736,-0.001954277926710889,-0.001384598358662863,-0.000916011846979873,-0.000593545407735683,-0.000429421385599475,-0.000407142410379183,-0.000489869695625105,-0.000630853460998336,-0.000783674244635851,-0.000910465431455086,-0.000986971617519803,-0.001004084647738694,-0.000966224936322048,-0.000887471604155053,-0.000786618128311534,-0.000682329334357303,-0.000589344418113718,-0.000516291531622168,-0.000465251321301040,-0.000432823344456239,-0.000412180921252613,-0.000395483070998306,-0.000376046667777760,-0.000349836665758677,-0.000316055860965474,-0.000276850226155611,-0.000236339821701050,-0.000199303342326874,-0.000169872539761481,-0.000150538332814806,-0.000141657313079940,-0.000141508706977206,-0.000146821594676434,-0.000153597634188172,-0.000158011427902620,-0.000157181343994892,-0.000149658359810178,-0.000135561336982934,-0.000116372847662614,-0.000094480871554409,-0.000072594827015514,-0.000053173935947240,-0.000037984418906137,-0.000027858477628469,-0.000022675017711940,-0.000021532467002142,-0.000023048146401550,-0.000025702247123743,-0.000028148219961627,-0.000029431397715943,-0.000029087039705276,-0.000027119763124050,-0.000023891471599733,-0.000019959728028917,-0.000015911433738929 };
const float32_t q_tx_coeffs[IQ_TX_HILBERT_TAPS] = { -0.000055876484573473,-0.000064501669527415,-0.000074738701316321,-0.000086507654429771,-0.000099410855699246,-0.000112742030021933,-0.000125581860328626,-0.000136976384505566,-0.000146171471747199,-0.000152855683517751,-0.000157351204077214,-0.000160693204323070,-0.000164554344882989,-0.000171001789764558,-0.000182113756522494,-0.000199522659221598,-0.000223982104007250,-0.000255065964662709,-0.000291093444701605,-0.000329333837166658,-0.000366484543520879,-0.000399347771170467,-0.000425571116339335,-0.000444281412941937,-0.000456442686257327,-0.000464813293654387,-0.000473459903203533,-0.000486891982756433,-0.000508986765583088,-0.000541954254458334,-0.000585620149319337,-0.000637266013271176,-0.000692159225019129,-0.000744745974185545,-0.000790300256973267,-0.000826662663506291,-0.000855608799816551,-0.000883393930312482,-0.000920144557182625,-0.000977999833391699,-0.001068208413135301,-0.001197698143396605,-0.001365882230232392,-0.001562573853813371,-0.001767799237590146,-0.001954010129822268,-0.002090729412517879,-0.002151093704598354,-0.002119197166650733,-0.001996721724352054,-0.001807182145055243,-0.001596303598977302,-0.001427605606562385,-0.001373133139886880,-0.001500318939184498,-0.001856985786393334,-0.002457280182694243,-0.003271661399515204,-0.004223805794000835,-0.005196381265502662,-0.006046183103054665,-0.006627310796927153,-0.006819219662039622,-0.006554969302166284,-0.005844167767895242,-0.004785243876864911,-0.003562886684106115,-0.002428691218878755,-0.001665958674417137,-0.001542756544570849,-0.002260180366086176,-0.003904692125323062,-0.006413957060137492,-0.009564477743523059,-0.012986526898586098,-0.016207713154723136,-0.018721568815726210,-0.020072626005615220,-0.019945430948836697,-0.018242650197742859,-0.015137438929630096,-0.011087811480348275,-0.006805694582534307,-0.003180041098991087,-0.001160864894184417,-0.001618136646873710,-0.005194938168183790,-0.012177070502025733,-0.022400779394307693,-0.035216238922248289,-0.049517325350430505,-0.063838956132037980,-0.076513206133968448,-0.085866090480225241,-0.090429824178616405,-0.089141744773038101,-0.081501626274434108,-0.067663896323307105,-0.048449702280469985,-0.025274673539499328,16.41584600088100E-15,0.025274673539530806,0.048449702280497546,0.067663896323328629,0.081501626274448152,0.089141744773044040,0.090429824178614490,0.085866090480216414,0.076513206133954376,0.063838956132020688,0.049517325350412138,0.035216238922230865,0.022400779394292886,0.012177070502014685,0.005194938168177043,0.001618136646871193,0.001160864894185542,0.003180041098994882,0.006805694582539598,0.011087811480353871,0.015137438929634959,0.018242650197746225,0.019945430948838137,0.020072626005614679,0.018721568815723948,0.016207713154719635,0.012986526898581976,0.009564477743518948,0.006413957060133944,0.003904692125320466,0.002260180366084725,0.001542756544570538,0.001665958674417785,0.002428691218880064,0.003562886684107731,0.004785243876866489,0.005844167767896496,0.006554969302167027,0.006819219662039779,0.006627310796926756,0.006046183103053836,0.005196381265501569,0.004223805793999667,0.003271661399514136,0.002457280182693402,0.001856985786392793,0.001500318939184273,0.001373133139886934,0.001427605606562642,0.001596303598977673,0.001807182145055637,0.001996721724352394,0.002119197166650973,0.002151093704598469,0.002090729412517876,0.001954010129822170,0.001767799237589989,0.001562573853813192,0.001365882230232221,0.001197698143396462,0.001068208413135197,0.000977999833391630,0.000920144557182582,0.000883393930312452,0.000855608799816521,0.000826662663506252,0.000790300256973217,0.000744745974185486,0.000692159225019068,0.000637266013271121,0.000585620149319296,0.000541954254458312,0.000508986765583088,0.000486891982756452,0.000473459903203566,0.000464813293654426,0.000456442686257364,0.000444281412941965,0.000425571116339349,0.000399347771170465,0.000366484543520863,0.000329333837166631,0.000291093444701573,0.000255065964662676,0.000223982104007221,0.000199522659221575,0.000182113756522479,0.000171001789764551,0.000164554344882987,0.000160693204323071,0.000157351204077214,0.000152855683517748,0.000146171471747192,0.000136976384505555,0.000125581860328611,0.000112742030021918,0.000099410855699231,0.000086507654429759,0.000074738701316315,0.000064501669527414,0.000055876484573478 };

//IIR RX/TX SSB IIR Elliptic highpass Fs=48000 Fpass=400hz Apass=1 Astop=80 // coefficients in reverse order than that spit out by MATLAB
const float32_t IIR_HPF_150_PKcoeffs[IIR_HPF_STAGES] = { 0.8118763975519,-0.9890897013525,0.998529467898,-0.998172360645,0.9999033698696,-0.9985778668215 };
const float32_t IIR_HPF_150_PVcoeffs[IIR_HPF_STAGES + 1] = { 0.8013501957703,-0.1732229209559,-0.02048512255242,0.0001540296081082,2.858519000881e-05,-5.689045501356e-08,-1.412260863809e-08 };

//IIR RX/TX SSB IIR Elliptic lowpass Fs=48000 Fpass=XXXX Apass=1 Astop=80 // coefficients in reverse order than that spit out by MATLAB
const float32_t IIR_LPF_0k3_PKcoeffs[IIR_LPF_CW_STAGES] = { -0.9645578265,0.9995203018,-0.9997460246,0.9997788072,-0.9997806549,0.9998542666,-0.9995376468 };
const float32_t IIR_LPF_0k3_PVcoeffs[IIR_LPF_CW_STAGES + 1] = { 1.691588659e-05,3.353849388e-05,9.358343505e-07,6.49710671e-07,7.28035543e-09,3.149453365e-09,1.187948004e-11,4.165954474e-12 };
const float32_t IIR_LPF_0k5_PKcoeffs[IIR_LPF_CW_STAGES] = { -0.9416306019,0.998668015,-0.9992944598,0.9993858337,-0.9993813038,0.9995949268,-0.998724699 };
const float32_t IIR_LPF_0k5_PVcoeffs[IIR_LPF_CW_STAGES + 1] = { 2.809501711e-05,5.595725816e-05,4.308294137e-06,3.027143521e-06,9.26890209e-08,4.03102689e-08,4.195671865e-10,1.46412521e-10 };
const float32_t IIR_LPF_1k4_PKcoeffs[IIR_LPF_STAGES] = { -0.8462825418,0.9897898436,-0.9945713878,0.995277524,-0.9954237938,0.9960355163,-0.9938547015,0.9986351728,-0.9877794385,0.9999324083,-0.9840809107 };
const float32_t IIR_LPF_1k4_PVcoeffs[IIR_LPF_STAGES + 1] = { 6.740443496e-05,0.0001428189862,5.744743612e-05,4.458257172e-05,7.519381143e-06,3.528056823e-06,2.807890667e-07,1.023707057e-07,2.650131004e-09,7.472060748e-10,4.456292974e-12,6.327882012e-13 };
const float32_t IIR_LPF_1k6_PKcoeffs[IIR_LPF_STAGES] = { -0.8263436556,0.9866844416,-0.9929075837,0.993832767,-0.9940218925,0.9948449731,-0.9920324683,0.9983254671,-0.983985126,0.9999088645,-0.9790742993 };
const float32_t IIR_LPF_1k6_PVcoeffs[IIR_LPF_STAGES + 1] = { 7.786652714e-05,0.0001696317486,8.620897279e-05,6.888441567e-05,1.461006013e-05,6.981711977e-06,7.024252113e-07,2.56179419e-07,8.289468667e-09,2.230242879e-09,1.682087282e-11,2.070473784e-12 };
const float32_t IIR_LPF_1k8_PKcoeffs[IIR_LPF_STAGES] = { -0.806874752,0.9831763506,-0.9910208583,0.9921959639,-0.9924327135,0.9934845567,-0.9898995161,0.9979222417,-0.9797343612,0.9998890758,-0.9734737277 };
const float32_t IIR_LPF_1k8_PVcoeffs[IIR_LPF_STAGES + 1] = { 8.879329835e-05,0.0001995324419,0.0001236492826,0.0001019445408,2.626318383e-05,1.280100332e-05,1.5726215e-06,5.741546829e-07,2.281308831e-08,5.87038107e-09,5.487214155e-11,5.410743034e-12 };
const float32_t IIR_LPF_2k1_PKcoeffs[IIR_LPF_STAGES] = { -0.7785284519,0.9771685004,-0.9877719283,0.9893809557,-0.9896966219,0.9911619425,-0.9861971736,0.9972315431,-0.9723653793,0.9998526573,-0.9639061093 };
const float32_t IIR_LPF_2k1_PVcoeffs[IIR_LPF_STAGES + 1] = { 0.0001062488955,0.0002513320069,0.0001992406615,0.0001726830815,5.665252684e-05,2.850408237e-05,4.491632808e-06,1.641089511e-06,8.445041288e-08,2.005996791e-08,2.554592649e-10,1.378373804e-11 };
const float32_t IIR_LPF_2k3_PKcoeffs[IIR_LPF_STAGES] = { -0.7601866126,0.9726730585,-0.9853261709,0.987264812,-0.9876374006,0.9894254208,-0.9833815098,0.9967161417,-0.966799438,0.99982512,-0.9567498565 };
const float32_t IIR_LPF_2k3_PVcoeffs[IIR_LPF_STAGES + 1] = { 0.0001187305534,0.0002913135977,0.0002649321104,0.0002377151104,8.925675502e-05,4.591491233e-05,8.314857041e-06,3.037660008e-06,1.809188035e-07,4.02478193e-08,6.222304338e-10,1.093283537e-11 };
const float32_t IIR_LPF_2k5_PKcoeffs[IIR_LPF_STAGES] = { -0.742277205,0.9677917361,-0.9826559424,0.9849573374,-0.9853895903,0.9875414968,-0.9802878499,0.9961705208,-0.9607117176,0.9997961521,-0.9489530325 };
const float32_t IIR_LPF_2k5_PVcoeffs[IIR_LPF_STAGES + 1] = { 0.0001320002921,0.0003363503201,0.0003449539945,0.0003206271795,0.0001354911365,7.129496953e-05,1.45802851e-05,5.32250624e-06,3.601761591e-07,7.397399315e-08,1.373902991e-09,-3.514191715e-11 };
const float32_t IIR_LPF_2k7_PKcoeffs[IIR_LPF_STAGES] = { -0.7247899175,0.9625301957,-0.979760766,0.9824588299,-0.982952714,0.9855135679,-0.9769080281,0.9955946207,-0.9540939927,0.9997653365,-0.9405293465 };
const float32_t IIR_LPF_2k7_PVcoeffs[IIR_LPF_STAGES + 1] = { 0.0001461596257,0.0003870833316,0.0004413111892,0.0004250271595,0.0001993143087,0.0001073063031,2.441546167e-05,8.898501619e-06,6.733910141e-07,1.253787048e-07,2.789394049e-09,-2.151171186e-10 };
const float32_t IIR_LPF_2k9_PKcoeffs[IIR_LPF_STAGES] = { -0.7077105045,0.9568932652,-0.97663939,0.9797691107,-0.9803255796,0.9833437204,-0.973236084,0.9949927926,-0.9469471574,0.99973315,-0.9314800501 };
const float32_t IIR_LPF_2k9_PVcoeffs[IIR_LPF_STAGES + 1] = { 0.0001613150234,0.0004441972414,0.0005562159349,0.000555051025,0.0002854237682,0.0001572258916,3.929471131e-05,1.428435644e-05,1.191940896e-06,1.962259688e-07,5.244403134e-09,-7.320882234e-10 };
const float32_t IIR_LPF_3k0_PKcoeffs[IIR_LPF_STAGES] = { -0.6993290186,0.9539387822,-0.9749951363,0.9783536792,-0.978941679,0.9822084308,-0.9712873697,0.994684577,-0.9431687593,0.9997166991,-0.9267210364 };
const float32_t IIR_LPF_3k0_PVcoeffs[IIR_LPF_STAGES + 1] = { 0.0001693023223,0.0004753801622,0.000621381856,0.0006311213947,0.0003385191667,0.0001886074751,4.918989725e-05,1.784992128e-05,1.556253324e-06,2.378592256e-07,6.995141799e-09,-1.231644564e-09 };
const float32_t IIR_LPF_3k2_PKcoeffs[IIR_LPF_STAGES] = { -0.6828542948,0.9477548599,-0.9715353251,0.975379169,-0.9760295749,0.9798349738,-0.9671613574,0.9940524101,-0.9352129698,0.9996827841,-0.9167417288 };
const float32_t IIR_LPF_3k2_PVcoeffs[IIR_LPF_STAGES + 1] = { 0.0001861676428,0.0005434747436,0.0007687733741,0.0008086091257,0.0004686490283,0.0002670602116,7.52732667e-05,2.719295298e-05,2.565597242e-06,3.261550034e-07,1.181958886e-08,-3.106660929e-09 };
const float32_t IIR_LPF_3k4_PKcoeffs[IIR_LPF_STAGES] = { -0.6667680144,0.9412142634,-0.9678487182,0.9722148776,-0.9729260802,0.9773304462,-0.9627223015,0.993404448,-0.9267144799,0.9996479154,-0.9061499834 };
const float32_t IIR_LPF_3k4_PVcoeffs[IIR_LPF_STAGES + 1] = { 0.0002043262793,0.000619910832,0.0009413009975,0.001024681842,0.0006365039153,0.000370805792,0.0001119176595,4.018983964e-05,4.057663318e-06,3.983414558e-07,1.863846322e-08,-6.992515456e-09 };
const float32_t IIR_LPF_3k6_PKcoeffs[IIR_LPF_STAGES] = { -0.6510609984,0.9343246222,-0.9639347196,0.9688612223,-0.9696301818,0.9746993184,-0.9579606652,0.9927446246,-0.9176692963,0.9996122718,-0.8949519992 };
const float32_t IIR_LPF_3k6_PVcoeffs[IIR_LPF_STAGES + 1] = { 0.0002239109599,0.0007055848255,0.00114207482,0.001285694889,0.000849928183,0.0005058745737,0.0001621705451,5.778811465e-05,6.178509466e-06,4.044969728e-07,2.73560623e-08,-1.443168074e-08 };
const float32_t IIR_LPF_3k8_PKcoeffs[IIR_LPF_STAGES] = { -0.6357243061,0.9270938635,-0.9597928524,0.9653186798,-0.9661409259,0.9719467163,-0.9528661966,0.9920771122,-0.9080735445,0.9995760918,-0.883153975 };
const float32_t IIR_LPF_3k8_PVcoeffs[IIR_LPF_STAGES + 1] = { 0.0002450637694,0.0008014678024,0.001374508953,0.001598743605,0.001117820386,0.0006792356144,0.0002295935701,8.102352876e-05,9.082836186e-06,2.577162377e-07,3.705021712e-08,-2.774891961e-08 };
const float32_t IIR_LPF_4k0_PKcoeffs[IIR_LPF_STAGES] = { -0.6207492352,0.9195302129,-0.955422461,0.9615877867,-0.9624569416,0.9690781236,-0.9474281073,0.9914060235,-0.8979237676,0.9995394945,-0.870762825 };
const float32_t IIR_LPF_4k0_PVcoeffs[IIR_LPF_STAGES + 1] = { 0.0002679372265,0.0009086125647,0.00164234417,0.001971707912,0.001450192998,0.0008988431073,0.0003182603105,0.0001109764125,1.291872559e-05,-1.802684721e-07,4.537296405e-08,-5.022265626e-08 };
const float32_t IIR_LPF_4k2_PKcoeffs[IIR_LPF_STAGES] = { -0.6061273217,0.9116423726,-0.9508231282,0.9576690793,-0.9585769176,0.9660995603,-0.941634655,0.9907354116,-0.8872167468,0.9995026588,-0.8577856421 };
const float32_t IIR_LPF_4k2_PVcoeffs[IIR_LPF_STAGES + 1] = { 0.0002926953021,0.001028161263,0.001949670259,0.002413296374,0.001858224277,0.001173664001,0.0004327296338,0.0001487074624,1.780556158e-05,-1.115183522e-06,4.777719909e-08,-8.617316638e-08 };
const float32_t IIR_LPF_4k4_PKcoeffs[IIR_LPF_STAGES] = { -0.5918501019,0.9034392834,-0.9459943175,0.9535631537,-0.9544990659,0.9630173445,-0.9354733229,0.9900693297,-0.8759495616,0.9994657636,-0.8442301154 };
const float32_t IIR_LPF_4k4_PVcoeffs[IIR_LPF_STAGES + 1] = { 0.0003195146273,0.001161353081,0.002300949302,0.00293308869,0.00235429802,0.001513683354,0.0005779900239,0.0001951702288,2.380468868e-05,-2.836876547e-06,3.659847891e-08,-1.408833157e-07 };
const float32_t IIR_LPF_4k6_PKcoeffs[IIR_LPF_STAGES] = { -0.5779095888,0.8949300647,-0.9409354925,0.9492706656,-0.9502215981,0.9598385692,-0.928930819,0.9894117713,-0.8641200066,0.9994290471,-0.8301042318 };
const float32_t IIR_LPF_4k6_PVcoeffs[IIR_LPF_STAGES + 1] = { 0.0003485856287,0.001309532556,0.002701039193,0.003541571787,0.002952031093,0.00192988175,0.0007593705086,0.0002510983322,3.088299491e-05,-5.734790193e-06,7.021868503e-11,-2.202550746e-07 };
const float32_t IIR_LPF_4k8_PKcoeffs[IIR_LPF_STAGES] = { -0.5642977953,0.8861243129,-0.9356461763,0.9447923899,-0.9457422495,0.9565707445,-0.9219930768,0.988766551,-0.8517261744,0.9993925691,-0.8154163361 };
const float32_t IIR_LPF_4k8_PVcoeffs[IIR_LPF_STAGES + 1] = { 0.0003801138664,0.00147415814,0.003155216575,0.004250175785,0.003666284028,0.002434180118,0.0009824144654,0.000316866237,3.887019921e-05,-1.03100092e-05,-7.860987239e-08,-3.300899039e-07 };
const float32_t IIR_LPF_5k0_PKcoeffs[IIR_LPF_STAGES] = { -0.5510069132,0.8770317435,-0.930126071,0.9401290417,-0.9410586357,0.9532220364,-0.9146451354,0.9881374836,-0.8387669325,0.9993565679,-0.8001753092 };
const float32_t IIR_LPF_5k0_PVcoeffs[IIR_LPF_STAGES + 1] = { 0.0004143215192,0.001656811684,0.003669201396,0.005071300548,0.004513151012,0.003039347939,0.001252712333,0.0003923241748,4.741162411e-05,-1.718107706e-05,-2.21988401e-07,-4.748961544e-07 };
const float32_t IIR_LPF_5k5_PKcoeffs[IIR_LPF_STAGES] = { -0.5191348791,0.8531115055,-0.9153136611,0.9276672602,-0.9284366369,0.944554925,-0.8943756819,0.9866580367,-0.8038932681,0.9992694855,-0.7597138286 };
const float32_t IIR_LPF_5k5_PVcoeffs[IIR_LPF_STAGES + 1] = { 0.000513187726,0.00220392202,0.005257628392,0.007706771139,0.00732680643,0.005083318334,0.002169718733,0.0006153709837,6.662667147e-05,-4.944315151e-05,-1.064442131e-06,-9.857741361e-07 };
const float32_t IIR_LPF_6k0_PKcoeffs[IIR_LPF_STAGES] = { -0.4891086817,0.8276233673,-0.8990523219,0.9140713811,-0.9144643545,0.9355916977,-0.871176064,0.9853506684,-0.765496552,0.9991881847,-0.7160081863 };
const float32_t IIR_LPF_6k0_PVcoeffs[IIR_LPF_STAGES + 1] = { 0.0006346812006,0.002905729227,0.007370365784,0.01136882883,0.01138002425,0.008063669316,0.003481954569,0.0008477381198,6.720562669e-05,-0.000114233997,-3.069567583e-06,-1.527193831e-06 };
const float32_t IIR_LPF_6k5_PKcoeffs[IIR_LPF_STAGES] = { -0.46082142,0.8007356524,-0.8813408017,0.8993632793,-0.8990678191,0.9265115261,-0.8447531462,0.984254837,-0.7236219645,0.9991143346,-0.6692284346 };
const float32_t IIR_LPF_6k5_PVcoeffs[IIR_LPF_STAGES + 1] = { 0.0007840811741,0.003799836617,0.01013919245,0.01633300819,0.01700467803,0.01219328586,0.005207507405,0.0009885194013,1.79684539e-05,-0.0002235489083,-6.763264082e-06,-1.416951932e-06 };
const float32_t IIR_LPF_7k0_PKcoeffs[IIR_LPF_STAGES] = { -0.4341723025,0.7726216912,-0.8621813059,0.8835719824,-0.882148087,0.917519331,-0.8147937655,0.9833983183,-0.6783508658,0.9990493059,-0.619559586 };
const float32_t IIR_LPF_7k0_PVcoeffs[IIR_LPF_STAGES + 1] = { 0.0009678382194,0.004931767937,0.01371903066,0.02290998586,0.02454460412,0.01763802022,0.007277265191,0.0008621326415,-0.000123870137,-0.0003789155453,-1.212523148e-05,7.398035677e-07 };
const float32_t IIR_LPF_7k5_PKcoeffs[IIR_LPF_STAGES] = { -0.4090664089,0.7434577346,-0.8415794373,0.8667365909,-0.8635750413,0.9088446498,-0.7809759378,0.982796073,-0.6298024058,0.9989940524,-0.5672011375 };
const float32_t IIR_LPF_7k5_PVcoeffs[IIR_LPF_STAGES + 1] = { 0.001193839125,0.006356356665,0.01828933321,0.03143793344,0.03432453051,0.02446442842,0.009496619925,0.0002152600646,-0.000403450802,-0.0005591837107,-1.791337854e-05,6.92005915e-06 };
const float32_t IIR_LPF_8k0_PKcoeffs[IIR_LPF_STAGES] = { -0.3854142129,0.7134215236,-0.8195450306,0.8489099741,-0.8431795835,0.900737226,-0.7429844737,0.982450366,-0.5781335831,0.9989491701,-0.5123662949 };
const float32_t IIR_LPF_8k0_PVcoeffs[IIR_LPF_STAGES + 1] = { 0.001471728669,0.008139299229,0.02405471541,0.04226968437,0.0466075018,0.03257810697,0.01151762903,-0.001260387595,-0.0008510519401,-0.0007097198395,-2.117295844e-05,1.894192428e-05 };
const float32_t IIR_LPF_8k5_PKcoeffs[IIR_LPF_STAGES] = { -0.3631313145,0.6826908588,-0.7960913777,0.8301638365,-0.8207443357,0.8934599161,-0.7005322576,0.9823516011,-0.5235373974,0.998914957,-0.4552812874 };
const float32_t IIR_LPF_8k5_PVcoeffs[IIR_LPF_STAGES + 1] = { 0.00181330007,0.01035887003,0.03124449216,0.05575376004,0.0615411289,0.04165901244,0.01283506677,-0.003869589884,-0.001459823572,-0.0007399945753,-1.742885433e-05,3.711246973e-05 };
const float32_t IIR_LPF_9k0_PKcoeffs[IIR_LPF_STAGES] = { -0.3421381116,0.6514419913,-0.7712346315,0.8105952144,-0.7959941626,0.8872777224,-0.6533867717,0.9824804664,-0.466239661,0.9988914728,-0.396184355 };
const float32_t IIR_LPF_9k0_PVcoeffs[IIR_LPF_STAGES + 1] = { 0.00223297067,0.01310778875,0.04011072591,0.0722084716,0.07909467071,0.05110361427,0.01282013953,-0.007816885598,-0.002165430924,-0.0005365324323,-1.932848591e-06,5.873909686e-05 };
const float32_t IIR_LPF_9k5_PKcoeffs[IIR_LPF_STAGES] = { -0.3223594725,0.6198486686,-0.744992137,0.7903344631,-0.7685856223,0.8824426532,-0.6013991237,0.9828101993,-0.4064949453,0.998878479,-0.3353245258 };
const float32_t IIR_LPF_9k5_PVcoeffs[IIR_LPF_STAGES + 1] = { 0.002748359693,0.01649521664,0.05092431605,0.09188860655,0.09899105877,0.0599870272,0.01080192532,-0.01308247168,-0.002837270033,5.808343758e-06,2.812779894e-05,7.740993897e-05 };
const float32_t IIR_LPF_10k0_PKcoeffs[IIR_LPF_STAGES] = { -0.3037244678,0.5880816579,-0.7173798084,0.7695555687,-0.7380974889,0.8791754246,-0.5445344448,0.9833092093,-0.3445818126,0.998875618,-0.272960633 };
const float32_t IIR_LPF_10k0_PVcoeffs[IIR_LPF_STAGES + 1] = { 0.003380991286,0.02064883709,0.06396860629,0.1149447113,0.1206404939,0.06706081331,0.006197689567,-0.01929922588,-0.003290334716,0.0009390416089,7.120672672e-05,8.396938938e-05 };
const float32_t IIR_LPF_15k0_PKcoeffs[IIR_LPF_STAGES] = { -0.1667787433,0.298262924,-0.3526200354,0.6261940598,-0.1301775426,0.9291864634,0.2007427812,0.9915837646,0.3234038949,0.9992539287,0.3721647561 };
const float32_t IIR_LPF_15k0_PVcoeffs[IIR_LPF_STAGES + 1] = { 0.02611710131,0.1489987522,0.3560487926,0.39397493,0.08741926402,-0.169127956,-0.04210657999,0.04211492836,0.00585997384,-0.001683055423,-0.0001497780177,-5.678643356e-05 };

//IIR HPF used for the FM noise squelch
const float32_t IIR_HPF_15k0_PKcoeffs[IIR_HPF_SQL_STAGES] = { 0.08584448365242,0.298455698329,0.6057782760185,0.7115173863477,0.7698117813539,0.7879540958152 };
const float32_t IIR_HPF_15k0_PVcoeffs[IIR_HPF_SQL_STAGES + 1] = { 0.00561889294652,-0.02629790457241,0.0684050306275,-0.1143219836396,0.1169486269215,-0.07287150832503,0.02534743920497 };

void InitAudioFilters(void)
{
	arm_fir_init_f32(&FIR_TX_Hilbert_I, IQ_TX_HILBERT_TAPS, (float32_t *)&i_tx_coeffs, (float32_t *)&Fir_Tx_Hilbert_State_I[0], APROCESSOR_BLOCK_SIZE); // +45 degrees phase added
	arm_fir_init_f32(&FIR_TX_Hilbert_Q, IQ_TX_HILBERT_TAPS, (float32_t *)&q_tx_coeffs, (float32_t *)&Fir_Tx_Hilbert_State_Q[0], APROCESSOR_BLOCK_SIZE); // -45 degrees phase added

	arm_iir_lattice_init_f32(&IIR_HPF_I, IIR_HPF_STAGES, (float32_t *)&IIR_HPF_150_PKcoeffs, (float32_t *)&IIR_HPF_150_PVcoeffs, (float32_t *)&IIR_HPF_State_I[0], APROCESSOR_BLOCK_SIZE);
	arm_iir_lattice_init_f32(&IIR_HPF_Q, IIR_HPF_STAGES, (float32_t *)&IIR_HPF_150_PKcoeffs, (float32_t *)&IIR_HPF_150_PVcoeffs, (float32_t *)&IIR_HPF_State_Q[0], APROCESSOR_BLOCK_SIZE);

	ReinitAudioLPFFilter();
	
	arm_iir_lattice_init_f32(&IIR_Squelch_HPF, IIR_HPF_SQL_STAGES, (float32_t *)&IIR_HPF_15k0_PKcoeffs, (float32_t *)&IIR_HPF_15k0_PVcoeffs, (float32_t *)&IIR_HPF_SQL_State[0], APROCESSOR_BLOCK_SIZE);
	
	InitNoiseReduction();
}

void ReinitAudioLPFFilter(void)
{
	if (CurrentVFO()->Filter_Width == 300)
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_CW_STAGES, (float32_t *)&IIR_LPF_0k3_PKcoeffs, (float32_t *)&IIR_LPF_0k3_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_CW_STAGES, (float32_t *)&IIR_LPF_0k3_PKcoeffs, (float32_t *)&IIR_LPF_0k3_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 500)
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_CW_STAGES, (float32_t *)&IIR_LPF_0k5_PKcoeffs, (float32_t *)&IIR_LPF_0k5_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_CW_STAGES, (float32_t *)&IIR_LPF_0k5_PKcoeffs, (float32_t *)&IIR_LPF_0k5_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 1400) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_1k4_PKcoeffs, (float32_t *)&IIR_LPF_1k4_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_1k4_PKcoeffs, (float32_t *)&IIR_LPF_1k4_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 1600) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_1k6_PKcoeffs, (float32_t *)&IIR_LPF_1k6_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_1k6_PKcoeffs, (float32_t *)&IIR_LPF_1k6_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 1800) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_1k8_PKcoeffs, (float32_t *)&IIR_LPF_1k8_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_1k8_PKcoeffs, (float32_t *)&IIR_LPF_1k8_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 2100) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_2k1_PKcoeffs, (float32_t *)&IIR_LPF_2k1_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_2k1_PKcoeffs, (float32_t *)&IIR_LPF_2k1_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 2300) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_2k3_PKcoeffs, (float32_t *)&IIR_LPF_2k3_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_2k3_PKcoeffs, (float32_t *)&IIR_LPF_2k3_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 2500) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_2k5_PKcoeffs, (float32_t *)&IIR_LPF_2k5_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_2k5_PKcoeffs, (float32_t *)&IIR_LPF_2k5_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 2700) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_2k7_PKcoeffs, (float32_t *)&IIR_LPF_2k7_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_2k7_PKcoeffs, (float32_t *)&IIR_LPF_2k7_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 2900) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_2k9_PKcoeffs, (float32_t *)&IIR_LPF_2k9_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_2k9_PKcoeffs, (float32_t *)&IIR_LPF_2k9_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 3000) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_3k0_PKcoeffs, (float32_t *)&IIR_LPF_3k0_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_3k0_PKcoeffs, (float32_t *)&IIR_LPF_3k0_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 3200) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_3k2_PKcoeffs, (float32_t *)&IIR_LPF_3k2_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_3k2_PKcoeffs, (float32_t *)&IIR_LPF_3k2_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 3400) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_3k4_PKcoeffs, (float32_t *)&IIR_LPF_3k4_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_3k4_PKcoeffs, (float32_t *)&IIR_LPF_3k4_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 3600) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_3k6_PKcoeffs, (float32_t *)&IIR_LPF_3k6_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_3k6_PKcoeffs, (float32_t *)&IIR_LPF_3k6_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 3800) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_3k8_PKcoeffs, (float32_t *)&IIR_LPF_3k8_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_3k8_PKcoeffs, (float32_t *)&IIR_LPF_3k8_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 4000) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_4k0_PKcoeffs, (float32_t *)&IIR_LPF_4k0_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_4k0_PKcoeffs, (float32_t *)&IIR_LPF_4k0_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 4200) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_4k2_PKcoeffs, (float32_t *)&IIR_LPF_4k2_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_4k2_PKcoeffs, (float32_t *)&IIR_LPF_4k2_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 4400) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_4k4_PKcoeffs, (float32_t *)&IIR_LPF_4k4_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_4k4_PKcoeffs, (float32_t *)&IIR_LPF_4k4_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 4600) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_4k6_PKcoeffs, (float32_t *)&IIR_LPF_4k6_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_4k6_PKcoeffs, (float32_t *)&IIR_LPF_4k6_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 4800) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_4k8_PKcoeffs, (float32_t *)&IIR_LPF_4k8_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_4k8_PKcoeffs, (float32_t *)&IIR_LPF_4k8_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 5000) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_5k0_PKcoeffs, (float32_t *)&IIR_LPF_5k0_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_5k0_PKcoeffs, (float32_t *)&IIR_LPF_5k0_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 5500) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_5k5_PKcoeffs, (float32_t *)&IIR_LPF_5k5_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_5k5_PKcoeffs, (float32_t *)&IIR_LPF_5k5_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 6000) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_6k0_PKcoeffs, (float32_t *)&IIR_LPF_6k0_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_6k0_PKcoeffs, (float32_t *)&IIR_LPF_6k0_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 6500) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_6k5_PKcoeffs, (float32_t *)&IIR_LPF_6k5_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_6k5_PKcoeffs, (float32_t *)&IIR_LPF_6k5_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 7000) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_7k0_PKcoeffs, (float32_t *)&IIR_LPF_7k0_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_7k0_PKcoeffs, (float32_t *)&IIR_LPF_7k0_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 7500) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_7k5_PKcoeffs, (float32_t *)&IIR_LPF_7k5_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_7k5_PKcoeffs, (float32_t *)&IIR_LPF_7k5_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 8000) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_8k0_PKcoeffs, (float32_t *)&IIR_LPF_8k0_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_8k0_PKcoeffs, (float32_t *)&IIR_LPF_8k0_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 8500) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_8k5_PKcoeffs, (float32_t *)&IIR_LPF_8k5_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_8k5_PKcoeffs, (float32_t *)&IIR_LPF_8k5_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 9000) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_9k0_PKcoeffs, (float32_t *)&IIR_LPF_9k0_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_9k0_PKcoeffs, (float32_t *)&IIR_LPF_9k0_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 9500) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_9k5_PKcoeffs, (float32_t *)&IIR_LPF_9k5_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_9k5_PKcoeffs, (float32_t *)&IIR_LPF_9k5_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 10000) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_10k0_PKcoeffs, (float32_t *)&IIR_LPF_10k0_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_10k0_PKcoeffs, (float32_t *)&IIR_LPF_10k0_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
	if (CurrentVFO()->Filter_Width == 15000) 
	{
		arm_iir_lattice_init_f32(&IIR_LPF_I, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_15k0_PKcoeffs, (float32_t *)&IIR_LPF_15k0_PVcoeffs, (float32_t *)&IIR_LPF_I_State[0], APROCESSOR_BLOCK_SIZE);
		arm_iir_lattice_init_f32(&IIR_LPF_Q, IIR_LPF_STAGES, (float32_t *)&IIR_LPF_15k0_PKcoeffs, (float32_t *)&IIR_LPF_15k0_PVcoeffs, (float32_t *)&IIR_LPF_Q_State[0], APROCESSOR_BLOCK_SIZE);
	}
}
