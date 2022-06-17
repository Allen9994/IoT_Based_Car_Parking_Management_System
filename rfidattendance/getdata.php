<?php  
//Connect to database
require 'connectDB.php';
date_default_timezone_set('Asia/Kolkata'); //Definiing timezone(For india)
$d = date("Y-m-d");
$t = date("H:i:s");

if (isset($_GET['card_uid']) && isset($_GET['device_token'])) { //When web-app recieves the UID and device token
    
    $card_uid = $_GET['card_uid'];  //Extracts UID and token to variable
    $device_uid = $_GET['device_token'];
    

    $sql = "SELECT * FROM devices WHERE device_uid=?";  //Checks validity of device token
    $result = mysqli_stmt_init($conn);
    if (!mysqli_stmt_prepare($result, $sql)) {
        echo "SQL_Error_Select_device";
        exit();
    }
    else{
        mysqli_stmt_bind_param($result, "s", $device_uid);
        mysqli_stmt_execute($result);
        $resultl = mysqli_stmt_get_result($result);
        if ($row = mysqli_fetch_assoc($resultl)){
            $device_mode = $row['device_mode'];
            $device_dep = $row['device_dep'];
            if ($device_mode == 1) {
                $sql = "SELECT * FROM users WHERE card_uid=?";  //Checks existence of UID in database
                $result = mysqli_stmt_init($conn);
                if (!mysqli_stmt_prepare($result, $sql)) {
                    echo "SQL_Error_Select_card";
                    exit();
                }
                else{
                    mysqli_stmt_bind_param($result, "s", $card_uid);
                    mysqli_stmt_execute($result);
                    $resultl = mysqli_stmt_get_result($result);
                    if ($row = mysqli_fetch_assoc($resultl)){
                        //*****************************************************
                        //An existed Card has been detected for Login or Logout
                        if ($row['add_card'] == 1){
                        if ($row['device_uid'] == $device_uid || $row['device_uid'] == 0){
                                $Uname = $row['username'];  //Extracts the name, serial number, User account balance and Telegram token and chat ID.
                                $Number = $row['serialnumber'];
                                $Amount = $row['amount'];
                                $Token = $row['tokenid'];
                                if ($Amount <= 10)  //Denies entry for the user if the balance is less than 10.
                                {
                                    echo "Low Balance"; //The response is sent back to NODE
                                    exit();
                                }
                                $sql = "SELECT * FROM users_logs WHERE card_uid=? AND checkindate=? AND card_out=0";
                                $result = mysqli_stmt_init($conn);
                                if (!mysqli_stmt_prepare($result, $sql)) {
                                    echo "SQL_Error_Select_logs";
                                    exit();
                                }
                                else{
                                    mysqli_stmt_bind_param($result, "ss", $card_uid, $d);
                                    mysqli_stmt_execute($result);
                                    $resultl = mysqli_stmt_get_result($result);
                                    //*****************************************************
                                    //Login
                                    if (!$row = mysqli_fetch_assoc($resultl)){  //When user is entering the parking lot
                                        $sql = "INSERT INTO users_logs (username, serialnumber, amount, card_uid, device_uid, device_dep, checkindate, timein, timeout) 
                                        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";
                                        $result = mysqli_stmt_init($conn);
                                        if (!mysqli_stmt_prepare($result, $sql)) {
                                            echo "SQL_Error_Select_login1";
                                            exit();
                                        }
                                        else{
                                            $timeout = "00:00:00";  //The timeout field is left 0 when user is entring the parking lot.
                                            mysqli_stmt_bind_param($result, "sdissssss", $Uname, $Number, $Amount, $card_uid, $device_uid, $device_dep, $d, $t, $timeout);
                                            mysqli_stmt_execute($result);   //Updates the userlog database with all the details of the user when admitting.

                                            echo $Amount."{0"."/".$Token."login".$Uname;
                                            exit();
                                        }
                                    }
                                    //*****************************************************
                                    //Logout
                                    else{   //When user is leaving the parking lot
                                        $sql = "SELECT timein FROM users_logs WHERE id=?";
                                        $result = mysqli_stmt_init($conn);
                                        $r = $row['timein'];    //Copies the entry time of user 
                                        $minute1 = date("i",strtotime($r)); //Extracts minute
                                        $second1 = date("s",strtotime($r)); //Extracts time
                                        $sql="UPDATE users_logs SET amount=?,timeout=?, card_out=1  WHERE card_uid=? AND checkindate=? AND card_out=0";
                                        $result = mysqli_stmt_init($conn);  //Updates the previous record of the user in userlog which was made when user entered the lot.
                                        if (!mysqli_stmt_prepare($result, $sql)) {  //Done by updating the timeout field and the account balance field.
                                            echo "SQL_Error_insert_logout1";
                                            exit();
                                        }
                                        else{
                                            $a = $minute1*60 + $second1;    //converts the entry time minute and second into integer value 
                                            $minute2 = date("i",strtotime($t)); //Extracts minutes and seconds of the current time (Time of exit)
                                            $second2 = date("s",strtotime($t));
                                            $b = $minute2*60 + $second2;    //Converts the exit time minute and second into integer value.
                                            $z = $b - $a;       //Finds the difference of the 2 time to extract the time spent inside the lot.
                                            if($z > 300)        //To find the charge based on time spent inside.
                                            {
                                                $c = $Amount - intdiv(($z-300),3) - 50; //When driver spends more than 5 minutes, for every 3 seconds, 1Rs is charged.
                                            }else
                                            if($z > 10) //When driver spends more than 10 seconds inside, for every 6 seconds, 1Rs is charged
                                            {
                                                $c = $Amount - intdiv($z,6);
                                            }
                                            else{          //Refuses to charge when the driver spends less than 10 seconds.
                                                    $c = $Amount;
                                                }

                                            mysqli_stmt_bind_param($result, "isss", $c, $t, $card_uid, $d);
                                            mysqli_stmt_execute($result);
                                            
                                            echo $c."{".$z."/".$Token."logout".$Uname;
                                            $sql="UPDATE users SET amount=? WHERE card_uid=?";      
                                            $result = mysqli_stmt_init($conn);
                                            if (!mysqli_stmt_prepare($result, $sql)) {
                                                exit();
                                            }
                                            else{
                                                mysqli_stmt_bind_param($result, "is", $c, $card_uid);   //Updates the time and balance of the previous record of the driver.
                                                mysqli_stmt_execute($result);
        
                                                exit();
                                            }
                                                                                       
                                            exit();
                                        }
                                    }
                                }
                            }
                            else {
                                echo "Not Allowed!";
                                exit();
                            }
                        }
                        else if ($row['add_card'] == 0){
                            echo "Not registerd!";  
                            exit();
                        }
                    }
                    else{
                        echo "Not found!";  //When the card is not registered.
                        exit();
                    }
                }
            }
            else if ($device_mode == 0) {
                //New Card has been added
                $sql = "SELECT * FROM users WHERE card_uid=?";
                $result = mysqli_stmt_init($conn);
                if (!mysqli_stmt_prepare($result, $sql)) {
                    echo "SQL_Error_Select_card";
                    exit();
                }
                else{
                    mysqli_stmt_bind_param($result, "s", $card_uid);
                    mysqli_stmt_execute($result);
                    $resultl = mysqli_stmt_get_result($result);
                    //The Card is available
                    if ($row = mysqli_fetch_assoc($resultl)){
                        $sql = "SELECT card_select FROM users WHERE card_select=1";
                        $result = mysqli_stmt_init($conn);
                        if (!mysqli_stmt_prepare($result, $sql)) {
                            echo "SQL_Error_Select";
                            exit();
                        }
                        else{
                            mysqli_stmt_execute($result);
                            $resultl = mysqli_stmt_get_result($result);
                            
                            if ($row = mysqli_fetch_assoc($resultl)) {
                                $sql="UPDATE users SET card_select=0";
                                $result = mysqli_stmt_init($conn);
                                if (!mysqli_stmt_prepare($result, $sql)) {
                                    echo "SQL_Error_insert";
                                    exit();
                                }
                                else{
                                    mysqli_stmt_execute($result);

                                    $sql="UPDATE users SET card_select=1, balance=? WHERE card_uid=?";
                                    $result = mysqli_stmt_init($conn);
                                    if (!mysqli_stmt_prepare($result, $sql)) {
                                        echo "SQL_Error_insert_An_available_card";
                                        exit();
                                    }
                                    else{
                                        mysqli_stmt_bind_param($result, "is", $balance, $card_uid);
                                        mysqli_stmt_execute($result);

                                        echo "available";
                                        exit();
                                    }
                                }
                            }
                            else{
                                $sql="UPDATE users SET card_select=1, WHERE card_uid=?";
                                $result = mysqli_stmt_init($conn);
                                if (!mysqli_stmt_prepare($result, $sql)) {
                                    echo "SQL_Error_insert_An_available_card";
                                    exit();
                                }
                                else{
                                    mysqli_stmt_bind_param($result, "s", $card_uid);
                                    mysqli_stmt_execute($result);

                                    echo "available";
                                    exit();
                                }
                            }
                        }
                    }
                    //The Card is new
                    else{
                        $sql="UPDATE users SET card_select=0";
                        $result = mysqli_stmt_init($conn);
                        if (!mysqli_stmt_prepare($result, $sql)) {
                            echo "SQL_Error_insert";
                            exit();
                        }
                        else{
                            mysqli_stmt_execute($result);
                            $sql = "INSERT INTO users (card_uid, card_select, device_uid, device_dep, user_date) VALUES (?, 1, ?, ?, CURDATE())";
                            $result = mysqli_stmt_init($conn);
                            if (!mysqli_stmt_prepare($result, $sql)) {
                                echo "SQL_Error_Select_add";
                                exit();
                            }
                            else{
                                mysqli_stmt_bind_param($result, "sss", $card_uid, $device_uid, $device_dep );
                                mysqli_stmt_execute($result);

                                echo "succesful";
                                exit();
                            }
                        }
                    }
                }    
            }
        }
        else{
            echo "Invalid Device!";
            exit();
        }
    }          
}
?>