<?php
require_once 'appinclude.php';

$friends = $facebook->api_client->fql_query("SELECT uid, political FROM 
user WHERE uid IN (SELECT uid2 FROM friend WHERE uid1=$user) ");

//Hardcoded possible categories in the spectrum
//TODO: Get rid of this and make it dynamically populated from what the friends list returns
$spectrum["Very Liberal"]=array();
$spectrum["Liberal"]=array();
$spectrum["Moderate"]=array();
$spectrum["Conservative"]=array();
$spectrum["Very Conservative"]=array();
$spectrum["Apathetic"]=array();
$spectrum["Libertarian"]=array();
$spectrum["Other"]=array();

//Return array [background color, text color]  given category title.  Result is string in hex or color name (suitable for CSS)
function categoryColor($category)
{
	switch($category)
	{
		case 'Very Liberal':
			return array('#941324', 'white');
			break;
		case 'Liberal':
			return array('#DC2039', 'black');
			break;
		case 'Moderate':
			return array('#F5F6FA', 'black');
			break;
		case 'Conservative':
			return array('#2A53C1', 'white');
			break;
		case 'Very Conservative':
			return array('#202A5D', 'white');
			break;
		case 'Apathetic':
			return array('Gainsboro', 'black');
			break;
		case 'Libertarian':
			return array('black', 'white');
			break;
		case 'Other':
			return array('DimGrey', 'black');
			break;			
	}
}

//Iterate through all friends and put them in the appropriate $spectrum[] array
foreach ($friends as $friend) 
{
	if ($friend["political"] != "") 
	{
		array_push($spectrum[$friend["political"]],  $friend["uid"]);
	}
}

$fbml = "";

$fbml .= '<center><table border="0" width="420">
    <tbody>
    <tr>
      <td>';

//Iterate through all friends in spectrum categories 	  
foreach ($spectrum as $type=>$friendsUIDs)
{
	if(count($friendsUIDs) > 0)
	{
		$numFriendsInType=0;
		$numFriendsToShow=5; //Friends in each type to show before collapse/hiding
		$color=categoryColor($type);
		$fbml .= '<h1 style="color: '.$color[1].'; background-color:'.$color[0].'; padding-left: 0.2em; ">' . $type . ' - '.count($spectrum[$type]).' Friends </h1>';

		//Iterate through all friends in type
		foreach($friendsUIDs as $friendUID)
		{
			$numFriendsInType += 1;
			if($numFriendsInType == $numFriendsToShow+1) //Show/hide code.  TODO:  Probably want this diabled on canvas page so we can see full thing (remove from canvas setFBML)
			//TODO:  For other people viewing your page, we are going to show orientations of all friends in common.  If you have lots of common friends this list will be huge.  But we can't count how many friends they have in common without an infinite sesion
			{
				$fbml .= '<fb:if-is-user uid="'.$user.'">';
				$numHiddenFriends=count($spectrum[$type]) - $numFriendsToShow;
				$fbml .= '<h4><br><a href="#" clicktotoggle="showNothing,expanded'.$type.'">Show '.$numHiddenFriends.' more '.$type.'\'s...</a></h4>';
				$fbml .= '<div id="showNothing"></div>';
				$fbml .= '<div id="expanded'.$type.'"  style="display:none;">';
			}
			
			//Emit the actual friends
			$fbml .= '<fb:if-is-friends-with-viewer uid="' . $friendUID . '">';
			$fbml .= '<fb:user-item uid="' . $friendUID . '"/>';
			$fbml .= '<table border="0" style="display: inline;">';
			$fbml .= '<tr><td valign="center"><fb:profile-pic uid="' . $friendUID . '"/></td></tr>';
			$fbml .= '<tr><td><fb:name uid="' . $friendUID . '"/></td></tr>';
			$fbml .= '</table>';
			$fbml .= "</fb:if-is-friends-with-viewer>";

		}
		if($numFriendsInType > $numFriendsToShow) //end clicktotoggle div
			$fbml .= '</div></fb:if-is-user>';

	}	
	$fbml .= '<br/><br/>';
}
$fbml .= '<br/>
      </td>
    </tr>
    </tbody>
  </table></center>';

$facebook->api_client->profile_setFBML($fbml, $user);

echo "<p>$fbml</p>";
//echo "<p>Hello, $user! asdfasdf</p>"
?>
