#ifndef _REGULATOR_INCLUDED_
#define _REGULATOR_INCLUDED_

#include "stm32f4xx.h"
#include "Path.h"

#define MAX_WHEEL_SPEED	    0.6// �/�
#define MAX_RAD_SPEED       2.7 // ���/�
#define LINE_SPEED_WEIGHT   0.8
#define ROTATE_SPEED_WEIGHT 1-LINE_SPEED_WEIGHT

#define JOYST_LIN_VEL_KOFF  MAX_WHEEL_SPEED/128.0
#define JOYST_TO_PWM        2.0
#define PHI_DEG             30
#define PHI_RAD             0.5236
#define distA	 	    0.31819
#define distB		    0.2625
#define RO                  (0.024)           // ������ �����
#define L                   (0.14)         // ���������� �� ������ ������ �� ��������� ������ ��������
#define JOYST_RAD_VEL_KOFF  MAX_RAD_SPEED/128.0 //(MAX_WHEEL_SPEED/distA/128.0/5.0)
#define DISKR_TO_REAL       (2.0*PI*RO/2800.0)
#define ONE_RO_COS_PHI      1.0                   // 23.0947
//#define COFF_TO_RAD         (2.0*PI/(4658.346666667))
#define MAX_CAPACITANCE     0.6 //(120.0*DISKR_TO_REAL/PID_PERIOD)*1.5   // ������������ ������ ������� �� ����
#define KOFF_ORTO_TRACE     0.9    // ����������� ����������� ������ ������������ ����������
#define PID_PERIOD          (1.0/100.0)

#define ENC_SET_CUR_POS   1
#define ENC_SET_GEOM   2

#define SEND_STATE   3

#define POINT_STACK_SIZE 20


typedef struct
{
  float p_k; //� ����������
  float i_k; //� ����������
  float d_k; //� ����������
  float target; //������� ��������
  float current; //������� (���������� ��������� ����� ����� ������ ��������)
  float prev_error; //���������� �������� ������ (��� � ����������)
  float sum_error; //��������� ������ (��� � ����������)
  float max_sum_error; //������������ ��������� ������ (��� �� � ��������� �� ������ �� ��������� ���� ���������� �������� ���������� ��������)
  float max_output; //������������ �����, ��� �� �������� �� �������� �� �����
  float min_output;
  float cut_output;
  float output; //��������, ��������� ������ ���
  char pid_on; //���/���� ��� ���� ���� �� output ������ ����� 0, ������ ��� ��������� ���������� �������������
  char pid_finish;
  float error_dir;
  float pid_error_end;
  float pid_output_end;
} PidStruct;


typedef struct
{
  float center [3];
  char (*movTask)(void);
  char (*endTask)(void);
  float endTaskP1;
  float (*speedVelTipe);
  float (*speedRotTipe);
  char step;
  float lengthTrace;
}pathPointStr;

extern Path curPath;
extern float Coord_local_track[3];
extern pathPointStr points[POINT_STACK_SIZE];
extern char lastPoint;
extern float normalVel[5];//V_���, V_���, V_���, �_���, �_����
extern float stopVel[5]; //{0.2,0.1,-0.05,0.2,0.7};
extern float standVel[5];

extern float normalRot[5];//V_���, V_���, V_���, �_���, �_����
extern float stopRot[5]; //{0.2,0.1,-0.1,0.3,0.6};
extern float standRot[5];
extern float * speedType[3];
extern float * rotType[3];
extern PidStruct wheelsPidStruct[4];
extern float regulatorOut[4];
extern uint16_t totalPointComplite;


void pidCalc(PidStruct *pid_control); //��������� ���, � �������� ��������� - ��������� �� ���������
void FunctionalRegulator(float *V_target, float *Coord_target, float *Coord_cur, float *V_out);
void pidWheelsFinishWait(void); // �������� ��������� ������������� ����� �����
void pidLowLevel(void); // ��� ������� ������ - ������
void GetDataForRegulators(void);
void infnormvect(float *a,char rows,float *b);
void Cost(float *inpMatr,char rows,float cost,float *outKoff);
void Regulate(float *Coord_err, float *Speed_cur, float *tAlphZad,float *V_etalon,float *alphZad, float *V_local);
void TrackRegulator(float *Coord_cur, float* speedCur, Path *cur, float *V);//void TrackRegulator(float *Coord_cur, float *Coord_center, float *Alph_zad, float *Phi_zad, float *V_etalon, float *V);
//void TrackRegulator(float *Coord_cur,Path *cur, float *V);
float linars(float *x, float *x0, float *x1);
void Moving(float Coord_x_cur, float Coord_x_targ, float* parameters, float* v_out);
void initRegulators(void);
void vectorAngle(float x, float y, float* angle);
void moving2(float Coord_x_cur, float Coord_x_targ, float* parameters, float* v_out);
signed char digitalize(char data, char lowerLevel, char upperLevel);
void CreatePath(pathPointStr * next_point, pathPointStr * cur_point, Path * out);
void removePoint(pathPointStr * points,char *lastPoint);



#endif
