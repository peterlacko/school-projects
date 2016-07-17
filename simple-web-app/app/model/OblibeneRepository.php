<?php
namespace Poradna;
use Nette;

/**
 *  * Tabulka oblibenych polozek
 *   */
class OblibeneRepository extends Repository {
	
    public function addFav($values) {
	    $this->connection->table('oblibene')->insert($values);
	    
    }

    public function deleteOblibene($id)
    {
	$this->findBy(array('ID_oblibene'=>$id))->delete();
    }
}
