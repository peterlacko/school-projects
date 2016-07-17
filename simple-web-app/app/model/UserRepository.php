<?php
namespace Poradna;
use Nette;

/**
 * Tabulka user
 */
class UserRepository extends Repository
{
    public function findByName($username)
    {
	return $this->findAll()->where('username', $username)->fetch();
    }

    public function addUser($username, $password, $fullname, $role = 'user') 
    {
	$this->connection->table('user')->insert(array(
	    'username'=>$username,
	    'password'=>Authenticator::calculateHash($password),
	    'fullname'=> $fullname,
	    'role'=>$role
	));
    }

    public function setPassword($id, $password)
    {
	$this->getTable()->where(array('user_id' => $id))->update(array(
	    'password' => Authenticator::calculateHash($password)
	));
    }

	public function deleteUser($id) 
	{
		$this->findBy(array('user_id' => $id))->delete();
	}
}
