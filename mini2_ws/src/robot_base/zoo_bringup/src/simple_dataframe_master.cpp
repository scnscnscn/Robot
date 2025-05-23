#include "simple_dataframe_master.h"
#include "data_holder.h"
#include <stdio.h>

#include "transport.h"

#include "base_driver.h"

Simple_dataframe::Simple_dataframe(Transport* _trans): trans(_trans){
    recv_state = STATE_RECV_FIX;
}

Simple_dataframe::~Simple_dataframe(){
}

bool Simple_dataframe::init(){
    trans->set_timeout(5000);
    return true;
}

bool Simple_dataframe::data_recv(unsigned char c){
    //printf("%02x ", c);
    switch (recv_state){
    case STATE_RECV_FIX:
        if (c == FIX_HEAD){
            memset(&active_rx_msg,0, sizeof(active_rx_msg));
            active_rx_msg.head.flag = c;
            active_rx_msg.check += c;

            recv_state = STATE_RECV_ID;
        }
        else
            recv_state = STATE_RECV_FIX;
        break;
    case STATE_RECV_ID:
        if (c < ID_MESSGAE_MAX){
            active_rx_msg.head.msg_id = c;
            active_rx_msg.check += c;
            recv_state = STATE_RECV_LEN;
        }
        else
            recv_state = STATE_RECV_FIX;
        break;
    case STATE_RECV_LEN:
        active_rx_msg.head.length =c;
        active_rx_msg.check += c;
        if (active_rx_msg.head.length==0)
            recv_state = STATE_RECV_CHECK;
        else
            recv_state = STATE_RECV_DATA;
        break;
    case STATE_RECV_DATA:
		//printf("%02x ", c);
        active_rx_msg.data[active_rx_msg.recv_count++] = c;
        active_rx_msg.check += c;
        if (active_rx_msg.recv_count >=active_rx_msg.head.length)
            recv_state  = STATE_RECV_CHECK;
        break;
    case STATE_RECV_CHECK:
        recv_state = STATE_RECV_FIX;
        if (active_rx_msg.check == c){
            //printf("\r\n");
            return true;
        }
        break;
    default:
        recv_state = STATE_RECV_FIX;
    }

    return false;
}

bool Simple_dataframe::data_parse(){
    MESSAGE_ID id = (MESSAGE_ID)active_rx_msg.head.msg_id;

    //printf("data_parse:id=%d\r\n", id);

    Data_holder* dh = Data_holder::get();
    switch (id){
    case ID_GET_VERSION:
        memcpy(&dh->firmware_info, active_rx_msg.data, sizeof(dh->firmware_info));
        break;
    case ID_SET_ROBOT_PARAMTER:
        break;
    case ID_GET_ROBOT_PARAMTER:
        memcpy(&dh->parameter, active_rx_msg.data, sizeof(dh->parameter));
        break;
    case ID_INIT_ODOM:
        break;
    case ID_SET_VELOCITY:
        break;
    case ID_GET_ODOM:
        memcpy(&dh->odom, active_rx_msg.data, sizeof(dh->odom));
        break;
    case ID_GET_PID_DATA:
        memcpy(&dh->pid_data, active_rx_msg.data, sizeof(dh->pid_data));
        break;
    case ID_GET_IMU_DATA:
        memcpy(&dh->imu_data, active_rx_msg.data, sizeof(dh->imu_data));
        break;
        //***********************************************************
    case ID_RESET_PLANT:
		//ROS_INFO_STREAM("8");
        break; 
    case ID_GET_PLANT:
        memcpy(&dh->highth, active_rx_msg.data, sizeof(dh->highth));
        break;
    case ID_SET_RGB:
        break;         
    case ID_GET_SENSOR:
    {
		memcpy(&dh->sensor, active_rx_msg.data, sizeof(dh->sensor));
		//printf("data_parse:sound1=%d   %d   sound2=%d   %d   sound3=%d   %d   sound4=%d   %d   sound5=%c   %c   sound6=%c   %c   \r\n", active_rx_msg.data[1], active_rx_msg.data[2], active_rx_msg.data[2], active_rx_msg.data[3], active_rx_msg.data[4], active_rx_msg.data[5], active_rx_msg.data[6], active_rx_msg.data[7], active_rx_msg.data[8], active_rx_msg.data[9], active_rx_msg.data[10], active_rx_msg.data[11]);
	}
        break;  
    case ID_ENABLE_MOTOR:
        break;  
    case ID_GET_VOLTAGE:
    {
		memcpy(&dh->voltage, active_rx_msg.data, sizeof(dh->voltage));
		//printf("data_parse:id=%-4s\r\n", (MESSAGE_ID)active_rx_msg.head.msg_id);
		//printf("data_parse:id=%d\r\n", id);
		//printf("data_parse:id=%-4s\r\n", active_rx_msg.head.length);
		//printf("data_parse:data=%c   %c   \r\n", active_rx_msg.data[1], active_rx_msg.data[2]);
		//printf("%s ", active_rx_msg.data);
	}
        break;       
        
        //************************************************************
    default:
        break;
    }

    return true;
}

bool Simple_dataframe::send_message(const MESSAGE_ID id){
    Message msg(id);

    send_message(&msg);

    return true;
}

bool Simple_dataframe::send_message(const MESSAGE_ID id, unsigned char* data, unsigned char len){
    Message msg(id, data, len);

    send_message(&msg);

    return true;
}

bool Simple_dataframe::send_message(Message* msg){
    if (trans == 0)
        return true;
    
    Buffer data((unsigned char*)msg, (unsigned char*)msg+sizeof(msg->head)+msg->head.length+1);
    trans->write(data);

    return true;
}

bool Simple_dataframe::interact(const MESSAGE_ID id){
	int i;
    //printf("make command:id=%d\r\n", id);

    Data_holder* dh = Data_holder::get();
    switch (id){
    case ID_GET_VERSION:
        send_message(id);
        break;
    case ID_SET_ROBOT_PARAMTER:
        send_message(id, (unsigned char*)&dh->parameter, sizeof(dh->parameter));
        break;
    case ID_GET_ROBOT_PARAMTER:
        send_message(id);
        break;
    case ID_INIT_ODOM:
        send_message(id);
        break;
    case ID_SET_VELOCITY:
        send_message(id, (unsigned char*)&dh->velocity, sizeof(dh->velocity));
        //ROS_INFO_STREAM("3");
        break;
    case ID_GET_ODOM:
        send_message(id);
        break;
    case ID_GET_PID_DATA:
        send_message(id);
        break;
    case ID_GET_IMU_DATA:
        send_message(id);
        break;
        
  /////////////////////////////////////////////      
    case ID_RESET_PLANT:
		//ROS_INFO_STREAM("8");
        send_message(id); 
        break; 
    case ID_GET_PLANT:
		//ROS_INFO_STREAM("9");
        send_message(id);  
        break;
    case ID_SET_RGB:
		//ROS_INFO_STREAM("10");
        send_message(id);
        break;         
    case ID_GET_SENSOR:
		//ROS_INFO_STREAM("11");
        send_message(id);
        break;  
    case ID_ENABLE_MOTOR:
		//ROS_INFO_STREAM("12");
        send_message(id);
        break;  
    case ID_GET_VOLTAGE:
		//ROS_INFO_STREAM("13");
        send_message(id);
        break;  
  /////////////////////////////////////////////////      
        
        
    default:
        break;
    }

    if (!recv_proc())
        return false;

    return true;
}

bool Simple_dataframe::recv_proc(){
    int i=0;
    trans->set_timeout(150);
    bool got=false;
    while(true){
        Buffer data = trans->read();
        
        for (int i=0;i<data.size();i++){
            if (data_recv(data[i])){
                got = true;
                //std::count << "ok" << std::endl;
                break;
            }
        }

        if (got)
            break;
        
        if (trans->is_timeout()){
            std::cout << "timeout:" << std::endl;
            return false;
        }
    }

    if (!data_parse())
        return false;

    return true;
}
