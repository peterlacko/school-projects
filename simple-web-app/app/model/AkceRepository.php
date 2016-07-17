<?php
namespace Poradna;
use Nette;

/**
 *  * Repozitar akci
 *   */
class AkceRepository extends Repository
{
    public function deleteAkce($id) {
	$this->findBy(array('ID_akce'=>$id))->delete();
    }

    public function addAkce($values) {
	$this->getTable()->insert(array(
	    'Titulek' => $values->titulek,
	    'Text' => $values->popis,
	    'Kdy' => $values->datum,
	    'ID_fakulty' => $values->fakulta
	));
    }

    public function search($phrase)
    {
		return $this->connection->table('akce')->where("Titulek LIKE ?", "%$phrase%");
    }
}
